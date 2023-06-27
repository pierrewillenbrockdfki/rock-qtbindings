require 'Qt5'
require 'test/unit'

class TestQtRuby < Test::Unit::TestCase

  def setup
    @app = Qt5::Application.instance || Qt5::Application.new(ARGV)
    assert @app
  end

  def test_link_against_qt5
    assert_raise(NoMethodError) { @app.setMainWidget(nil) }
  end

  def test_qapplication_methods
   assert @app == Qt5::Application::instance
   assert @app == Qt5::CoreApplication::instance
   assert @app == Qt5::Application.instance
   assert @app == Qt5::CoreApplication.instance
   assert @app == $qApp
  end

  def test_qapplication_inheritance
   assert @app.inherits("Qt5::Application")
   assert @app.inherits("Qt5::CoreApplication")
   assert @app.inherits("Qt5::Object")
  end

  def test_widget_inheritance
    widget = Qt5::Widget.new(nil)
    assert widget.inherits("Qt5::Widget")
    assert widget.inherits("Qt5::Object")
    assert widget.inherits("QObject")
  end

  def test_qstring_marshall
    widget = Qt5::Widget.new(nil)
    assert widget.objectName.nil?
    widget.objectName = "Barney"
    assert widget.objectName == "Barney"
  end

  def test_widgetlist
    w1 = Qt5::Widget.new(nil)
    w2 = Qt5::Widget.new(w1)
    w3 = Qt5::Widget.new(w1)
    w4 = Qt5::Widget.new(w2)

    assert w1.children == [ w2, w3 ]
  end

  def test_find_children
    w = Qt5::Widget.new(nil)
    assert_raise(TypeError) { w.findChildren(nil) }

    assert w.findChildren(Qt5::Widget) == [ ]
    w2 = Qt5::Widget.new(w)

    assert w.findChildren(Qt5::Widget) == [ w2 ]
    assert w.findChildren(Qt5::Object) == [ w2 ]
    assert w.findChildren(Qt5::LineEdit) == [ ]
    assert w.findChildren(Qt5::Widget,"Bob") == [ ]
    assert w.findChildren(Qt5::Object,"Bob") == [ ]

    w2.objectName = "Bob"

    assert w.findChildren(Qt5::Widget) == [ w2 ]
    assert w.findChildren(Qt5::Object) == [ w2 ]
    assert w.findChildren(Qt5::Widget,"Bob") == [ w2 ]
    assert w.findChildren(Qt5::Object,"Bob") == [ w2 ]
    assert w.findChildren(Qt5::LineEdit, "Bob") == [ ]

    w3 = Qt5::Widget.new(w)
    w4 = Qt5::LineEdit.new(w2)
    w4.setObjectName("Bob")

    assert w.findChildren(Qt5::Widget) == [ w4, w2, w3 ]
    assert w.findChildren(Qt5::LineEdit) == [ w4 ]
    assert w.findChildren(Qt5::Widget,"Bob") == [ w4, w2 ]    
    assert w.findChildren(Qt5::LineEdit,"Bob") == [ w4 ]    
  end

  def test_find_child
    w = Qt5::Widget.new(nil)
    assert_raise(TypeError) { w.findChild(nil) }

    assert_nil w.findChild(Qt5::Widget)
    w2 = Qt5::Widget.new(w)

    w3 = Qt5::Widget.new(w)
    w3.objectName = "Bob"
    w4 = Qt5::LineEdit.new(w2)
    w4.objectName = "Bob"

    assert w.findChild(Qt5::Widget,"Bob") == w3
    assert w.findChild(Qt5::LineEdit,"Bob") == w4
  end

  def test_boolean_marshalling
    assert Qt5::Variant.new(true).toBool
    assert !Qt5::Variant.new(false).toBool

    assert !Qt5::Boolean.new(true).nil?
    assert Qt5::Boolean.new(false).nil?

    # Invalid variant conversion should change b to false
    b = Qt5::Boolean.new(true)
    v = Qt5::Variant.new("Blah")
    v.toInt(b);

    assert b.nil?
  end

  def test_intp_marshalling
    assert Qt5::Integer.new(100).value == 100
  end

  def test_variant_conversions
    v = Qt5::Variant.new(Qt5::Variant::Invalid)

    assert !v.isValid
    assert v.isNull

    v = Qt5::Variant.new(55)
    assert v.toInt == 55
    assert v.toUInt == 55
    assert v.toLongLong == 55
    assert v.toULongLong == 55
    assert Qt5::Variant.new(-55).toLongLong == -55
    assert Qt5::Variant.new(-55).toULongLong == 18446744073709551561
    assert v.toDouble == 55.0
    assert v.toChar == Qt5::Char.new(55)
    assert v.toString == "55"
    assert v.toStringList == [ ]


    assert Qt5::Variant.new("Blah").toStringList == [ "Blah" ]

    assert Qt5::Variant.new(Qt5::Size.new(30,40)).toSize == Qt5::Size.new(30,40)
    assert Qt5::Variant.new(Qt5::SizeF.new(20,30)).toSizeF == Qt5::SizeF.new(20,30)

    assert Qt5::Variant.new(Qt5::Rect.new(30,40,10,10)).toRect == Qt5::Rect.new(30,40,10,10)
    assert Qt5::Variant.new(Qt5::RectF.new(20,30,10,10)).toRectF == Qt5::RectF.new(20,30,10,10)

    assert Qt5::Variant.new(Qt5::Point.new(30,40)).toPoint == Qt5::Point.new(30,40)
    assert Qt5::Variant.new(Qt5::PointF.new(20,30)).toPointF == Qt5::PointF.new(20,30)


  end

end
