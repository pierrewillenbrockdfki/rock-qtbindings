#!/usr/bin/env ruby -w

# This is the EventFilter example from Chapter 16 of 'Programming with Qt'

require 'Qt5'

class KillerFilter < Qt5::Object

	def eventFilter( object, event )
		if event.type() == Qt5::Event::MouseButtonPress
			if event.button() == Qt5::RightButton
				object.close()
				return true
			else
				return false
			end
		else
			return false
		end
	end

end
	
a = Qt5::Application.new(ARGV)
	
toplevel = Qt5::Widget.new
toplevel.resize(230, 130)

killerfilter = KillerFilter.new

pb = Qt5::PushButton.new(toplevel)
pb.setGeometry( 10, 10, 100, 50 )
pb.text = "pushbutton"
pb.installEventFilter(killerfilter)

le = Qt5::LineEdit.new(toplevel)
le.setGeometry( 10, 70, 100, 50 )
le.text = "Line edit"
le.installEventFilter(killerfilter)

cb = Qt5::CheckBox.new(toplevel)
cb.setGeometry( 120, 10, 100, 50 )
cb.text = "Check-box"
cb.installEventFilter(killerfilter)

rb = Qt5::RadioButton.new(toplevel)
rb.setGeometry( 120, 70, 100, 50 )
rb.text = "Radio button"
rb.installEventFilter(killerfilter)

toplevel.show
a.exec


	
