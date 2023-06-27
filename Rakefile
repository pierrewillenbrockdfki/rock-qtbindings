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
      file.write("QT5BINDINGS_VERSION = '#{ENV['VERSION']}'\n")
      file.write("QT5BINDINGS_RELEASE_DATE = '#{Time.now}'\n")
    end
    File.open('qtlib/qt5bindings_qt_version.rb', 'w') do |file|
      file.write("QT5BINDINGS_QT_VERSION = '#{ENV['VERSION']}'\n")
      file.write("QT5BINDINGS_QT_RELEASE_DATE = '#{Time.now}'\n")
    end
  end
end

def clear_version
  if ENV['VERSION']
    File.open('lib/qt5bindings_version.rb', 'w') do |file|
      file.write("QT5BINDINGS_VERSION = '0.0.0.0'\n")
      file.write("QT5BINDINGS_RELEASE_DATE = ''\n")
    end
    File.open('qtlib/qt5bindings_qt_version.rb', 'w') do |file|
      file.write("QT5BINDINGS_QT_VERSION = '0.0.0.0'\n")
      file.write("QT5BINDINGS_QT_RELEASE_DATE = ''\n")
    end
  end
end

task :build_examples do
  # Go into the examples directory and look for all the makefiles and build them
  Dir['examples/**/makefile'].each do |file|
    if windows
      system("cd #{File.dirname(file).gsub('/', '\\')} && #{MAKE}")
    else
      system("cd #{File.dirname(file)} && #{MAKE}")
    end
  end
end

task :examples => [:build_examples] do
  system('cd examples && ruby run_all.rb')
end

task :default => [:all]

task :extconf do
  system('ruby extconf.rb')
end

# All calls 'make clean' and 'make build'
task :all => [:extconf] do
  system("#{MAKE} all")
end

task :clean => [:extconf] do
  system("#{MAKE} clean")
end

task :distclean => [:extconf] do
  system("#{MAKE} distclean")
end

task :make_build => [:extconf] do
  system("#{MAKE} build")
end

task :install => [:extconf] do
  system("#{MAKE} install")
end

task :gem => [:distclean] do
  warn_version()
  set_version()
  system("gem build qt5bindings.gemspec")
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
  system("#{MAKE} installqt5")
  system("#{COPY} gemspecs#{SLASH}qt5bindings-qt5.gemspec .")
  system("gem build qt5bindings-qt5.gemspec")
  system("#{DEL} qt5bindings-qt5.gemspec")
  clear_version()
end

task :build do
  Rake::Task[:extconf].execute
  Rake::Task[:all].execute
  Rake::Task[:install].execute
end
