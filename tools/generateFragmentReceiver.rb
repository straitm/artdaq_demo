
# Generate the FHiCL document which configures the FragmentGenerator class

require File.join( File.dirname(__FILE__), 'demo_utilities' )

def generateFragmentReceiver(startingFragmentId, boardId, 
                             fragmentType, configDoc )

  generator = nil
  if fragmentType == "V1720" || fragmentType == "V1724"
    generator = "V172xSimulator"
  elsif fragmentType == "TOY1" || fragmentType == "TOY2"
    generator = "ToySimulator"
  elsif fragmentType == "UDP"
    generator = "UDPReceiver"
  elsif fragmentType == "ASCII"
    generator = "AsciiSimulator"
  end

  if configDoc == nil
    configDoc = generator + ".fcl"
  end

  fgConfig = String.new( read_fcl("CommandableFragmentGenerator.fcl") + "\
    generator: %{generator}
    fragment_type: %{fragment_type}
    fragment_id: %{starting_fragment_id}
    board_id: %{board_id}
    starting_fragment_id: %{starting_fragment_id}
    random_seed: %{random_seed}
    sleep_on_stop_us: 500000 " \
    + read_fcl(configDoc) )
  
  fgConfig.gsub!(/\%\{generator\}/, String(generator))
  fgConfig.gsub!(/\%\{fragment_type\}/, String(fragmentType)) 
  fgConfig.gsub!(/\%\{starting_fragment_id\}/, String(startingFragmentId))
  fgConfig.gsub!(/\%\{board_id\}/, String(boardId))
  fgConfig.gsub!(/\%\{random_seed\}/,String(rand(10000)))

  return fgConfig

end
