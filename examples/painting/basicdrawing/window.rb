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
	
	
require './renderarea.rb'	
	
class Window < Qt5::Widget
	
	slots   'shapeChanged()',
    		'penChanged()',
    		'brushChanged()'
	
	def initialize(parent = nil)
		super
	 	@idRole = Qt5::UserRole
	    @renderArea = RenderArea.new
	
	    @shapeComboBox = Qt5::ComboBox.new do |s|
			s.addItem(tr("Rectangle"), Qt5::Variant.new(RenderArea::Rect))
			s.addItem(tr("Round Rectangle"), Qt5::Variant.new(RenderArea::RoundRect))
			s.addItem(tr("Ellipse"), Qt5::Variant.new(RenderArea::Ellipse))
			s.addItem(tr("Pie"), Qt5::Variant.new(RenderArea::Pie))
			s.addItem(tr("Chord"), Qt5::Variant.new(RenderArea::Chord))
			s.addItem(tr("Polygon"), Qt5::Variant.new(RenderArea::Polygon))
			s.addItem(tr("Path"), Qt5::Variant.new(RenderArea::Path))
			s.addItem(tr("Line"), Qt5::Variant.new(RenderArea::Line))
			s.addItem(tr("Polyline"), Qt5::Variant.new(RenderArea::Polyline))
			s.addItem(tr("Arc"), Qt5::Variant.new(RenderArea::Arc))
			s.addItem(tr("Points"), Qt5::Variant.new(RenderArea::Points))
			s.addItem(tr("Text"), Qt5::Variant.new(RenderArea::Text))
			s.addItem(tr("Pixmap"), Qt5::Variant.new(RenderArea::Pixmap))
		end
	
	    @shapeLabel = Qt5::Label.new(tr("&Shape:"))
	    @shapeLabel.buddy = @shapeComboBox
	
	    @penWidthSpinBox = Qt5::SpinBox.new
	    @penWidthSpinBox.range = 0..20
	
	    @penWidthLabel = Qt5::Label.new(tr("Pen &Width:"))
	    @penWidthLabel.buddy = @penWidthSpinBox
	
	    @penStyleComboBox = Qt5::ComboBox.new do |p|
			p.addItem(tr("Solid"), Qt5::Variant.new(Qt5::SolidLine.to_i))
			p.addItem(tr("Dash"), Qt5::Variant.new(Qt5::DashLine.to_i))
			p.addItem(tr("Dot"), Qt5::Variant.new(Qt5::DotLine.to_i))
			p.addItem(tr("Dash Dot"), Qt5::Variant.new(Qt5::DashDotLine.to_i))
			p.addItem(tr("Dash Dot Dot"), Qt5::Variant.new(Qt5::DashDotDotLine.to_i))
			p.addItem(tr("None"), Qt5::Variant.new(Qt5::NoPen.to_i))
		end
	
	    @penStyleLabel = Qt5::Label.new(tr("&Pen Style:"))
	    @penStyleLabel.buddy = @penStyleComboBox
	
	    @penCapComboBox = Qt5::ComboBox.new do |p|
	    	p.addItem(tr("Flat"), Qt5::Variant.new(Qt5::FlatCap.to_i))
	    	p.addItem(tr("Square"), Qt5::Variant.new(Qt5::SquareCap.to_i))
	    	p.addItem(tr("Round"), Qt5::Variant.new(Qt5::RoundCap.to_i))
		end
	
	    @penCapLabel = Qt5::Label.new(tr("Pen &Cap:"))
	    @penCapLabel.buddy = @penCapComboBox
	
	    @penJoinComboBox = Qt5::ComboBox.new do |p|
	    	p.addItem(tr("Miter"), Qt5::Variant.new(Qt5::MiterJoin.to_i))
	    	p.addItem(tr("Bevel"), Qt5::Variant.new(Qt5::BevelJoin.to_i))
	    	p.addItem(tr("Round"), Qt5::Variant.new(Qt5::RoundJoin.to_i))
		end
	
	    @penJoinLabel = Qt5::Label.new(tr("Pen &Join:"))
	    @penJoinLabel.buddy = @penJoinComboBox
	
	    @brushStyleComboBox = Qt5::ComboBox.new do |b|
			b.addItem(tr("Linear Gradient"),
					Qt5::Variant.new(Qt5::LinearGradientPattern.to_i))
			b.addItem(tr("Radial Gradient"),
					Qt5::Variant.new(Qt5::RadialGradientPattern.to_i))
			b.addItem(tr("Conical Gradient"),
					Qt5::Variant.new(Qt5::ConicalGradientPattern.to_i))
			b.addItem(tr("Texture"), Qt5::Variant.new(Qt5::TexturePattern.to_i))
			b.addItem(tr("Solid"), Qt5::Variant.new(Qt5::SolidPattern.to_i))
			b.addItem(tr("Horizontal"), Qt5::Variant.new(Qt5::HorPattern.to_i))
			b.addItem(tr("Vertical"), Qt5::Variant.new(Qt5::VerPattern.to_i))
			b.addItem(tr("Cross"), Qt5::Variant.new(Qt5::CrossPattern.to_i))
			b.addItem(tr("Backward Diagonal"), Qt5::Variant.new(Qt5::BDiagPattern.to_i))
			b.addItem(tr("Forward Diagonal"), Qt5::Variant.new(Qt5::FDiagPattern.to_i))
			b.addItem(tr("Diagonal Cross"), Qt5::Variant.new(Qt5::DiagCrossPattern.to_i))
			b.addItem(tr("Dense 1"), Qt5::Variant.new(Qt5::Dense1Pattern.to_i))
			b.addItem(tr("Dense 2"), Qt5::Variant.new(Qt5::Dense2Pattern.to_i))
			b.addItem(tr("Dense 3"), Qt5::Variant.new(Qt5::Dense3Pattern.to_i))
			b.addItem(tr("Dense 4"), Qt5::Variant.new(Qt5::Dense4Pattern.to_i))
			b.addItem(tr("Dense 5"), Qt5::Variant.new(Qt5::Dense5Pattern.to_i))
			b.addItem(tr("Dense 6"), Qt5::Variant.new(Qt5::Dense6Pattern.to_i))
			b.addItem(tr("Dense 7"), Qt5::Variant.new(Qt5::Dense7Pattern.to_i))
			b.addItem(tr("None"), Qt5::Variant.new(Qt5::NoBrush.to_i))
		end
	
	    @brushStyleLabel = Qt5::Label.new(tr("&Brush Style:"))
	    @brushStyleLabel.buddy = @brushStyleComboBox
	
	    @antialiasingCheckBox = Qt5::CheckBox.new(tr("&Antialiasing"))
	    @transformationsCheckBox = Qt5::CheckBox.new(tr("&Transformations"))
	
	    connect(@shapeComboBox, SIGNAL('activated(int)'),
	            self, SLOT('shapeChanged()'))
	    connect(@penWidthSpinBox, SIGNAL('valueChanged(int)'),
	            self, SLOT('penChanged()'))
	    connect(@penStyleComboBox, SIGNAL('activated(int)'),
	            self, SLOT('penChanged()'))
	    connect(@penCapComboBox, SIGNAL('activated(int)'),
	            self, SLOT('penChanged()'))
	    connect(@penJoinComboBox, SIGNAL('activated(int)'),
	            self, SLOT('penChanged()'))
	    connect(@brushStyleComboBox, SIGNAL('activated(int)'),
	            self, SLOT('brushChanged()'))
	    connect(@antialiasingCheckBox, SIGNAL('toggled(bool)'),
	            @renderArea, SLOT('antialiased=(bool)'))
	    connect(@transformationsCheckBox, SIGNAL('toggled(bool)'),
	            @renderArea, SLOT('transformed=(bool)'))
	
	    checkBoxLayout = Qt5::HBoxLayout.new do |c|
	    	c.addWidget(@antialiasingCheckBox)
	    	c.addWidget(@transformationsCheckBox)
		end
	
	    self.layout = Qt5::GridLayout.new do |l|
			l.addWidget(@renderArea, 0, 0, 1, 2)
			l.addWidget(@shapeLabel, 1, 0)
			l.addWidget(@shapeComboBox, 1, 1)
			l.addWidget(@penWidthLabel, 2, 0)
			l.addWidget(@penWidthSpinBox, 2, 1)
			l.addWidget(@penStyleLabel, 3, 0)
			l.addWidget(@penStyleComboBox, 3, 1)
			l.addWidget(@penCapLabel, 4, 0)
			l.addWidget(@penCapComboBox, 4, 1)
			l.addWidget(@penJoinLabel, 5, 0)
			l.addWidget(@penJoinComboBox, 5, 1)
			l.addWidget(@brushStyleLabel, 6, 0)
			l.addWidget(@brushStyleComboBox, 6, 1)
			l.addLayout(checkBoxLayout, 7, 0, 1, 2)
		end
	
	    shapeChanged()
	    penChanged()
	    brushChanged()
	    @renderArea.antialiased = false
	    @renderArea.transformed = false
	
	    setWindowTitle(tr("Basic Drawing"))
	end
	
	def shapeChanged()
	    shape = @shapeComboBox.itemData(@shapeComboBox.currentIndex(), @idRole).toInt
	    @renderArea.shape = shape
	end
	
	def penChanged()
	    width = @penWidthSpinBox.value()
	    style = @penStyleComboBox.itemData(@penStyleComboBox.currentIndex(), @idRole).toInt
	    cap = @penCapComboBox.itemData(@penCapComboBox.currentIndex(), @idRole).toInt
	    join = @penJoinComboBox.itemData(@penJoinComboBox.currentIndex(), @idRole).toInt
	
	    @renderArea.pen = Qt5::Pen.new(Qt5::Brush.new(Qt5::blue), width, style, cap, join)
	end
	
	def brushChanged()
	    style = @brushStyleComboBox.itemData(@brushStyleComboBox.currentIndex(), @idRole).toInt
	
	    if style == Qt5::LinearGradientPattern
	        linearGradient = Qt5::LinearGradient.new(0, 0, 100, 100)
	        linearGradient.setColorAt(0.0, Qt5::Color.new(Qt5::white))
	        linearGradient.setColorAt(0.2, Qt5::Color.new(Qt5::green))
	        linearGradient.setColorAt(1.0, Qt5::Color.new(Qt5::black))
	        @renderArea.brush = Qt5::Brush.new(linearGradient)
	    elsif style == Qt5::RadialGradientPattern
	        radialGradient = Qt5::RadialGradient.new(50, 50, 50, 50, 50)
	        radialGradient.setColorAt(0.0, Qt5::Color.new(Qt5::white))
	        radialGradient.setColorAt(0.2, Qt5::Color.new(Qt5::green))
	        radialGradient.setColorAt(1.0, Qt5::Color.new(Qt5::black))
	        @renderArea.brush = Qt5::Brush.new(radialGradient)
	    elsif style == Qt5::ConicalGradientPattern
	        conicalGradient = Qt5::ConicalGradient.new(50, 50, 150)
	        conicalGradient.setColorAt(0.0, Qt5::Color.new(Qt5::white))
	        conicalGradient.setColorAt(0.2, Qt5::Color.new(Qt5::green))
	        conicalGradient.setColorAt(1.0, Qt5::Color.new(Qt5::black))
	        @renderArea.brush = Qt5::Brush.new(conicalGradient)
	    elsif style == Qt5::TexturePattern
	        @renderArea.brush = Qt5::Brush.new(Qt5::Pixmap.new("images/brick.png"))
	    else
	        @renderArea.brush = Qt5::Brush.new(Qt5::green, style)
	    end
	end
end
