#!/bin/bash
if [ ! -d "icons" ]; then
  mkdir icons
fi
if [ ! -d "icons/86x86" ]; then
  mkdir icons/86x86
fi
if [ ! -d "icons/108x108" ]; then
  mkdir icons/108x108
fi
if [ ! -d "icons/128x128" ]; then
  mkdir icons/128x128
fi
if [ ! -d "icons/256x256" ]; then
  mkdir icons/256x256
fi

convert -background none -resize 86x86 harbour-cover-miner.svg icons/86x86/harbour-cover-miner.png
convert -background none -resize 108x108 harbour-cover-miner.svg icons/108x108/harbour-cover-miner.png
convert -background none -resize 128x128 harbour-cover-miner.svg icons/128x128/harbour-cover-miner.png
convert -background none -resize 256x256 harbour-cover-miner.svg icons/256x256/harbour-cover-miner.png