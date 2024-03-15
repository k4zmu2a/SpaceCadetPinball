cask "space-cadet-pinball" do
  version "2.1.0"
  sha256 "2cfd5ab18154ed05880591a63ee97cbe877b5da47949ba0e228dc0bf17f3932a"

  url "https://github.com/k4zmu2a/SpaceCadetPinball/releases/download/Release_#{version}/SpaceCadetPinballMac.zip"
  name "SpaceCadetPinball"
  desc "3D Pinball for Windows - Space Cadet"
  homepage "https://github.com/k4zmu2a/SpaceCadetPinball"

  livecheck do
    url :url
    strategy :github_latest
  end

  app "SpaceCadetPinball.app"

  zap trash: "~/Library/Application Support/SpaceCadetPinball/"
end
