#!/usr/bin/ruby

module Qt5UiTools
    module Internal
        def self.init_all_classes
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
