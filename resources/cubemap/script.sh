#!/bin/bash

input_image="$1"

# Based on the visible image, each face appears to be approximately 512x512 pixels
# Let's make sure we get clean cuts at the boundaries
face_size=256
face_width=192
# Extract each face according to the visible layout:
# Top (grass field with fence)
convert "$input_image" -crop ${face_size}x${face_size}+256+0 front.png

# Left (grass field with trees on horizon) 
convert "$input_image" -crop ${face_size}x${face_size}+0+256 left.png

# Front (grass field center)
convert "$input_image" -crop ${face_size}x${face_size}+256+256 bottom.png

# Right (grass field with sun in sky)
convert "$input_image" -crop ${face_size}x${face_size}+512+256 right.png

# Bottom (grass field with trees reflection)
convert "$input_image" -crop ${face_size}x${face_size}+256+512 back.png

# Back (grass field with sky)
convert "$input_image" -crop ${face_size}x${face_size}+768+256 top.png

echo "Extracted all 6 faces of the cubemap."