=begin
/***************************************************************************
                          qtscript.rb  -  QtScript ruby client lib
                             -------------------
    begin                : 11-07-2008
    copyright            : (C) 2008 by Richard Dale
    email                : richard.j.dale@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
=end

module Qt5Script
  module Internal
    def self.init_all_classes
#      Qt5::Internal::add_normalize_proc(Proc.new do |classname|
#        if classname =~ /^Qt5Script/
#          now = classname.sub(/^Qt5Script?(?=[A-Z])/,'Qt5Script::')
#        end
#        now
#      end)
      getClassList.each do |c|
        classname = Qt5::Internal::normalize_classname(c)
        id = Qt5::Internal::findClass(c);
        Qt5::Internal::insert_pclassid(classname, id)
        Qt5::Internal::cpp_names[classname] = c
        klass = Qt5::Internal::isQObject(c) ? Qt5::Internal::create_qobject_class(classname, Qt5) \
                                           : Qt5::Internal::create_qt_class(classname, Qt5)
        Qt5::Internal::classes[classname] = klass unless klass.nil?
      end
    end
  end
end
