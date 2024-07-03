=begin
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**

** Translated to QtRuby by Richard Dale
=end
	
	
	
class MdiChild < Qt5::TextEdit
	
	attr_reader :currentFile

	slots 'documentWasModified()'
	
	def initialize()
		super
	    setAttribute(Qt5::WA_DeleteOnClose)
	    @isUntitled = true
	end
	
	def newFile()
	    @@sequenceNumber = 1
	
	    @isUntitled = true
	    @currentFile = tr("document%s.txt" % @@sequenceNumber += 1)
	    setWindowTitle(@currentFile + "[*]")
	
	    connect(document(), SIGNAL('contentsChanged()'),
	            self, SLOT('documentWasModified()'))
	end
	
	def loadFile(fileName)
	    file = Qt5::File.new(fileName)
	    if !file.open(Qt5::IODevice::ReadOnly | Qt5::IODevice::Text)
	        Qt5::MessageBox.warning(self, tr("MDI"),
	                             tr("Cannot read file %s:\n%s." % [fileName, file.errorString]))
	        return false
	    end
	
	    inf = Qt5::TextStream.new(file)
	    Qt5::Application.overrideCursor = Qt5::Cursor.new(Qt5::WaitCursor)
	    setPlainText(inf.readAll())
	    Qt5::Application.restoreOverrideCursor()
	
	    setCurrentFile(fileName)
	
	    connect(document(), SIGNAL('contentsChanged()'),
	            self, SLOT('documentWasModified()'))
	
	    return true
	end
	
	def save()
	    if @isUntitled
	        return saveAs()
	    else
	        return saveFile(@currentFile)
	    end
	end
	
	def saveAs()
	    fileName = Qt5::FileDialog.getSaveFileName(self, tr("Save As"),
	                                                    @currentFile)
	    if not fileName or fileName.empty?
	        return false
		end
	
	    return saveFile(fileName)
	end
	
	def saveFile(fileName)
	    file = Qt5::File.new(fileName)
	    if !file.open(Qt5::IODevice::WriteOnly | Qt5::IODevice::Text)
	        Qt5::MessageBox::warning(self, tr("MDI"),
	                             tr("Cannot write file %s:\n%s." % [fileName, file.errorString]))
	        return false
	    end
	
	    outf = Qt5::TextStream.new(file)
	    Qt5::Application.setOverrideCursor(Qt5::WaitCursor)
	    outf << toPlainText()
	    Qt5::Application.restoreOverrideCursor()
	
	    setCurrentFile(fileName)
	    return true
	end
	
	def userFriendlyCurrentFile()
	    return strippedName(@currentFile)
	end
	
	def closeEvent(event)
	    if maybeSave()
	        event.accept()
	    else
	        event.ignore()
	    end
	end
	
	def documentWasModified()
	    setWindowModified(document().isModified())
	end
	
	def maybeSave()
	    if document().isModified()
	        ret = Qt5::MessageBox::warning(self, tr("MDI"),
	                     tr("'%s' has been modified.\n" \
	                        "Do you want to save your changes?" % 
                              userFriendlyCurrentFile()),
	                     Qt5::MessageBox::Yes | Qt5::MessageBox::Default,
	                     Qt5::MessageBox::No,
	                     Qt5::MessageBox::Cancel | Qt5::MessageBox::Escape)
	        if ret == Qt5::MessageBox::Yes
	            return save()
	        elsif ret == Qt5::MessageBox::Cancel
	            return false
			end
	    end
	    return true
	end
	
	def setCurrentFile(fileName)
	    @currentFile = Qt5::FileInfo.new(fileName).canonicalFilePath()
	    @isUntitled = false
	    document().modified = false
	    setWindowModified(false)
	    setWindowTitle(userFriendlyCurrentFile() + "[*]")
	end
	
	def strippedName(fullFileName)
	    return Qt5::FileInfo.new(fullFileName).fileName()
	end
end
