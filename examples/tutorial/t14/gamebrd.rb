require_relative 'lcdrange.rb'
require_relative 'cannon.rb'

class GameBoard < Qt5::Widget
  slots 'fire()', 'hit()', 'missed()', 'newGame()'

  def initialize()
    super
    quit = Qt5::PushButton.new('&Quit')
    quit.font = Qt5::Font.new('Times', 18, Qt5::Font::Bold)

    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))

    angle = LCDRange.new('ANGLE')
    angle.range = 5..70

    force = LCDRange.new('FORCE')
    force.range = 10..50

    cannonBox = Qt5::Frame.new
    cannonBox.frameStyle = Qt5::Frame::WinPanel | Qt5::Frame::Sunken

    @cannonField = CannonField.new

    connect(angle, SIGNAL('valueChanged(int)'),
            @cannonField, SLOT('angle=(int)'))
    connect(@cannonField, SIGNAL('angleChanged(int)'),
            angle, SLOT('value=(int)'))

    connect(force, SIGNAL('valueChanged(int)'),
            @cannonField, SLOT('force=(int)'))
    connect(@cannonField, SIGNAL('forceChanged(int)'),
            force, SLOT('value=(int)'))

    connect(@cannonField, SIGNAL('hit()'),
                self, SLOT('hit()'))
    connect(@cannonField, SIGNAL('missed()'),
                self, SLOT('missed()'))

    shoot = Qt5::PushButton.new('&Shoot')
    shoot.font = Qt5::Font.new('Times', 18, Qt5::Font::Bold)

    connect(shoot, SIGNAL('clicked()'), SLOT('fire()'))
    connect(@cannonField, SIGNAL('canShoot(bool)'),
                shoot, SLOT('setEnabled(bool)'))

    restart = Qt5::PushButton.new('&New Game')
    restart.font = Qt5::Font.new('Times', 18, Qt5::Font::Bold)

    connect(restart, SIGNAL('clicked()'), self, SLOT('newGame()'))

    @hits = Qt5::LCDNumber.new(2, self)
    @shotsLeft = Qt5::LCDNumber.new(2, self)
    hitsLabel = Qt5::Label.new('HITS', self)
    shotsLeftLabel = Qt5::Label.new('SHOTS LEFT', self)

    Qt5::Shortcut.new(Qt5::KeySequence.new(Qt5::Key_Enter), self, SLOT('fire()'))
    Qt5::Shortcut.new(Qt5::KeySequence.new(Qt5::Key_Return), self, SLOT('fire()'))
    Qt5::Shortcut.new(Qt5::KeySequence.new(Qt5::CTRL + Qt5::Key_Q), self, SLOT('close()'))

    topLayout = Qt5::HBoxLayout.new
    topLayout.addWidget(shoot)
    topLayout.addWidget(@hits)
    topLayout.addWidget(hitsLabel)
    topLayout.addWidget(@shotsLeft)
    topLayout.addWidget(shotsLeftLabel)
    topLayout.addStretch(1)
    topLayout.addWidget(restart)

    leftLayout = Qt5::VBoxLayout.new()
    leftLayout.addWidget(angle)
    leftLayout.addWidget(force)

    cannonLayout = Qt5::VBoxLayout.new
    cannonLayout.addWidget(@cannonField)
    cannonBox.layout = cannonLayout

    gridLayout = Qt5::GridLayout.new
    gridLayout.addWidget(quit, 0, 0)
    gridLayout.addLayout(topLayout, 0, 1)
    gridLayout.addLayout(leftLayout, 1, 0)
    gridLayout.addWidget(cannonBox, 1, 1, 2, 1)
    gridLayout.setColumnStretch(1, 10)
    setLayout(gridLayout)

    angle.value = 60
    force.value = 25
    angle.setFocus

    newGame()
  end

  def fire()
    if @cannonField.gameOver || @cannonField.shooting?
      return
    end
    @shotsLeft.display(@shotsLeft.intValue() - 1)
    @cannonField.shoot
  end

  def hit()
    @hits.display(@hits.intValue() + 1)
    if @shotsLeft.intValue() == 0
      @cannonField.setGameOver
    else
      @cannonField.newTarget
    end
  end

  def missed()
    if @shotsLeft.intValue() == 0
      @cannonField.setGameOver
    end
  end

  def newGame()
    @shotsLeft.display(15.0)
    @hits.display(0)
    @cannonField.restartGame
    @cannonField.newTarget
  end
end
