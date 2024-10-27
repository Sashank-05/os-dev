from PIL import Image
import numpy as np

def gif_to_monochrome_c_array(gif_path):
    # Open the GIF file
    gif = Image.open(gif_path)
    
    frame_number = 0
    c_arrays = []
    
    # Iterate over each frame in the GIF
    while True:
        try:
            # Extract the current frame
            gif.seek(frame_number)
            
            # Convert frame to monochrome (1-bit)
            monochrome_frame = gif.convert('1')
            
            # Convert the monochrome image to a NumPy array (0 or 1 for each pixel)
            frame_data = np.array(monochrome_frame)
            
            # Get frame dimensions
            width, height = monochrome_frame.size
            
            # Convert the frame data to a C array
            c_array = bmp_to_c_array(frame_data)
            c_arrays.append((c_array, width, height))
            
            frame_number += 1
        
        except EOFError:
            # End of GIF
            break
    
    return c_arrays

def bmp_to_c_array(frame_data):
    c_frame = []
    
    for row in frame_data:
        # Convert each row to a comma-separated string of 0s and 1s
        row_data = ', '.join(map(str, row))
        c_frame.append(f'{{ {row_data} }}')
    
    # Join all rows together
    return '{\n    ' + ',\n    '.join(c_frame) + '\n}'

# Convert all frames to C arrays
gif_path = 'animation.gif'  # Path to your GIF
c_arrays = gif_to_monochrome_c_array(gif_path)

# Format all frames into the desired 3D C array format
num_frames = len(c_arrays)
print(f"Number of frames: {num_frames}")

# Generate the C code with dimensions
c_code = f"int frames[{num_frames}][<height>][<width>] = {{\n"
for i, (c_array, width, height) in enumerate(c_arrays):
    print(f"Frame {i} dimensions: {width}x{height}")
    c_code += f"    // Frame {i}\n    {c_array},\n"

c_code += "};"

# Output the final C array code
print(c_code)
