require File.expand_path('lib/qt5bindings_version', __dir__)

spec = Gem::Specification.new do |s|
  s.authors = ['Ryan Melton', 'Jason Thomas', 'Richard Dale', 'Arno Rehn']
  s.email = 'kde-bindings@kde.org'
  s.rubyforge_project = 'qt5bindings'
  s.summary = "Qt5 bindings for ruby"
  s.homepage = "http://github.com/pierrewillenbrockdfki/qt5bindings"
  s.name = 'qt5bindings'
  s.version = QT5BINDINGS_VERSION
  s.requirements << 'none'
  s.require_path = 'lib'
  s.files = Dir['lib/**/*', 'bin/**/*', 'examples/**/*', 'ext/**/*', '*.txt', 'extconf.rb', '*.gemspec', 'Rakefile'].to_a
  s.extensions = ['extconf.rb']
  s.executables = ['smoke5api', 'smoke5deptool', 'rbrcc5', 'rbuic5', 'rbqt5api']
  s.description = 'qt5bindings provides ruby bindings to QT5.x. It is derived from the kdebindings project.'
  s.licenses = ['LGPL-2.1']
end
