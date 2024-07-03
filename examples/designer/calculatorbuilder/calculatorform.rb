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
	
class CalculatorForm < Qt5::Widget
		
	slots	'on_inputSpinBox1_valueChanged(int)',
    		'on_inputSpinBox2_valueChanged(int)'
	
	def initialize(parent = nil)
	    super(parent)
	    loader = Qt5::UiLoader.new
	
	    file = Qt5::File.new(":/forms/calculatorform.ui")
	    file.open(Qt5::File::ReadOnly)
	    formWidget = loader.load(file, self)
	    file.close
	
	    @ui_inputSpinBox1 = findChild(Qt5::SpinBox, "inputSpinBox1")
	    @ui_inputSpinBox2 = findChild(Qt5::SpinBox, "inputSpinBox2")
	    @ui_outputWidget = findChild(Qt5::Label, "outputWidget")
	
	    Qt5::MetaObject.connectSlotsByName(self)
	
	    self.layout = Qt5::VBoxLayout.new do |l|
	    	l.addWidget(formWidget)
		end
	
	    self.windowTitle = tr("Calculator Builder")
	end
	
	def on_inputSpinBox1_valueChanged(value)
	    @ui_outputWidget.text = (value + @ui_inputSpinBox2.value).to_s
	end
	
	def on_inputSpinBox2_valueChanged(value)
	    @ui_outputWidget.text = (value + @ui_inputSpinBox1.value).to_s
	end
end
