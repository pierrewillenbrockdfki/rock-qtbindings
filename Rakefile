require 'rake'

windows = false
processor, platform, *rest = RUBY_PLATFORM.split("-")
windows = true if platform =~ /mswin32/ or platform =~ /mingw32/

if windows
  MAKE = 'mingw32-make'
  SLASH = '\\'
  COPY = 'copy'
  DEL = 'del'
else
  MAKE = 'make'
  SLASH = '/'
  COPY = 'cp'
  DEL = 'rm'
end

def warn_version
  puts 'Warning: VERSION not specified' unless ENV['VERSION']
end

def set_version
  if ENV['VERSION']
    File.open('lib/qt5bindings_version.rb', 'w') do |file|
      file.write("QTBINDINGS_VERSION = '#{ENV['VERSION']}'\n")
      file.write("QTBINDINGS_RELEASE_DATE = '#{Time.now}'\n")
    end
    File.open('qtlib/qt5bindings_qt_version.rb', 'w') do |file|
      file.write("QTBINDINGS_QT_VERSION = '#{ENV['VERSION']}'\n")
      file.write("QTBINDINGS_QT_RELEASE_DATE = '#{Time.now}'\n")
    end
  end
end

def clear_version
  if ENV['VERSION']
    File.open('lib/qt5bindings_version.rb', 'w') do |file|
      file.write("QTBINDINGS_VERSION = '0.0.0.0'\n")
      file.write("QTBINDINGS_RELEASE_DATE = ''\n")
    end
    File.open('qtlib/qt5bindings_qt_version.rb', 'w') do |file|
      file.write("QTBINDINGS_QT_VERSION = '0.0.0.0'\n")
      file.write("QTBINDINGS_QT_RELEASE_DATE = ''\n")
    end
  end
end

task :build_examples do
  # Go into the examples directory and look for all the makefiles and build them
  Dir['examples/**/makefile'].each do |file|
    if windows
      sh("cd #{File.dirname(file).gsub('/', '\\')} && #{MAKE}")
    else
      sh("cd #{File.dirname(file)} && #{MAKE}")
    end
  end
end

task :examples => [:build_examples] do
  sh('cd examples && ruby run_all.rb')
end

task :default => [:all]

task :extconf do
  sh('ruby extconf.rb')
end

# All calls 'make clean' and 'make build'
task :all => [:extconf] do
  sh("#{MAKE} all")
end

task :clean => [:extconf] do
  sh("#{MAKE} clean")
end

task :distclean => [:extconf] do
  sh("#{MAKE} distclean")
end

task :make_build => [:extconf] do
  sh("#{MAKE} build")
end

task :install => [:extconf] do
  sh("#{MAKE} install")
end

task :gem => [:distclean] do
  warn_version()
  set_version()
  sh("gem build qt5bindings.gemspec")
  clear_version()
end

task :gemnative do
  warn_version()
  set_version()
  system("#{COPY} gemspecs#{SLASH}qt5bindingsnative.gemspec .")
  system("gem build qt5bindingsnative.gemspec")
  system("#{DEL} qt5bindingsnative.gemspec")
  clear_version()
end

task :gemqt do
  warn_version()
  set_version()
  system("#{MAKE} installqt")
  system("#{COPY} gemspecs#{SLASH}qt5bindings-qt.gemspec .")
  system("gem build qt5bindings-qt.gemspec")
  system("#{DEL} qt5bindings-qt.gemspec")
  clear_version()
end

task :build do
  Rake::Task[:extconf].execute
  Rake::Task[:all].execute
  Rake::Task[:install].execute
end
