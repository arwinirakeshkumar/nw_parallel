#!/usr/bin/ruby
charset = %w{A G C T}
puts (0...ARGV[0].to_i).map{ charset.to_a[rand(charset.size)] }.join
puts (0...ARGV[1].to_i).map{ charset.to_a[rand(charset.size)] }.join
