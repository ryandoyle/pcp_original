Gem::Specification.new do |s|
  s.name    = "pcp"
  s.version = "0.0.1"
  s.summary = "PCP API for Ruby"
  s.author  = "Ryan Doyle"

  s.files = Dir.glob("ext/**/*.{c,rb}") +
            Dir.glob("lib/**/*.rb")

  s.extensions << "ext/pcp_native/extconf.rb"

  s.add_development_dependency "rake-compiler"
  s.add_development_dependency "rspec"
end