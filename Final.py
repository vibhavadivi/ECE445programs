import serial
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
from matplotlib.offsetbox import OffsetImage, AnnotationBbox
import numpy as np

x0 = 0
y0 = 0

x7 = 0
y7 = 2.65

x8 = 3.2
y8 = 0

xlim = max(x0, x7, x8) + 1
ylim = max(y0, y7, y8) + 1

# Initialize the plot
plt.ion()  # Turn on interactive mode
fig, ax = plt.subplots()
fig.subplots_adjust(bottom=0.2, right=0.8)  # Adjust the bottom margin to make room for the image

# Set the limits for the x and y axes
ax.set_xlim(-1, xlim)
ax.set_ylim(-1, ylim)

ax_acc = fig.add_axes([0.85, 0.2, 0.1, 0.6])
ax_acc.axis('off')
acc_text = ax_acc.text(0.5, 0.5, '', ha='center', va='center', fontsize=16)


# Load the images
football_img = mpimg.imread('football.png')  # Load the football image
field_img = mpimg.imread('football_field.png')  # Load the football field image

# Set the field image as the background of the plot
ax.imshow(field_img, extent=[-1, xlim, -1, ylim])
current_football = None
current_annotation = None
# Initialize the serial connection
ser = serial.Serial('COM12', 115200)

d0_list = []
d7_list = []
d8_list = []

x= 0
y=0

num_dists = 7

AccX_val = 100
AccY_val = 100
AccZ_val = 100
GyrX_val = 0
GyrY_val = 0
GyrZ_val = 0

while True:
    line = ser.readline().decode('ascii').strip()  # Read a line from the serial port and decode it from bytes to string
    if line.startswith('from: '):  # Check if the line starts with 'from: '
        # if abs(AccX_val) < 0.6 and abs(AccY_val) < 0.6 and abs(AccZ_val) < 0.6:
        #     continue
        parts = line.split('\t')  # Split the line into two parts using the tab character
        address = int(parts[0].split(': ')[1], 16)  # Extract the address from the first part of the line
        distance = float(parts[1].split(': ')[1].split(' ')[0])  # Extract the distance from the second part of the line
        #print((address, distance))  # Print the tuple of (address, distance)
        if address == 128:
            d0_list.append(distance)
        elif address == 135:
            d7_list.append(distance)
        elif address == 136:
            d8_list.append(distance)

        if len(d0_list) >= num_dists and len(d7_list) >= num_dists and len(d8_list) >= num_dists:
            d0 = np.mean(d0_list)
            d7 = np.mean(d7_list)
            d8 = np.mean(d8_list)
            d0_list = []
            d7_list = []
            d8_list = []
            A = 2*x7 - 2*x0
            B = 2*y7 - 2*y0
            C = d0 * d0 - d7 * d7 - x0 * x0 + x7 * x7 - y0 * y0 + y7 * y7
            D = 2 * x8 - 2 * x7
            E = 2 * y8 - 2 * y7
            F = d7 * d7 - d8 * d8 - x7 * x7 + x8 * x8 - y7 * y7 + y8 * y8
            x = (C * E - F*B)/(E * A - B * D)
            y= (C * D - A * F) / (B * D - A * E)
            print(x, y)

            if current_football:
                current_football.remove()

            if current_annotation:
                current_annotation.remove()


            # Update the position of the football image
            img = OffsetImage(football_img, zoom=0.05)  # Create an image object for the football image
            ab = AnnotationBbox(img, (x, y), xycoords='data', frameon=False)  # Create an annotation box for the football image
            current_football = ax.add_artist(ab)  # Add the annotation box to the plot

            # Annotate the coordinates of the football image
            current_annotation = ax.annotate('({:.2f}, {:.2f})'.format(x, y), (x, y), textcoords='offset points', xytext=(10, 10), fontsize=12)
            # Redraw the plot
            fig.canvas.draw_idle()

            # Wait for a short time to allow the plot to be updated
            plt.pause(.1)
    elif line.startswith("AccX: "):
        values = line.split(',')
        AccX_val = float(values[0].split(': ')[1])
        AccY_val = float(values[1].split(': ')[1])
        AccZ_val = float(values[2].split(': ')[1])
        GyrX_val = float(values[3].split(': ')[1])
        GyrY_val = float(values[4].split(': ')[1])
        GyrZ_val = float(values[5].split(': ')[1])
        acc_text.set_text('AccX: {:.2f}\nAccY: {:.2f}\nAccZ: {:.2f}\nGyrX: {:.2f}\nGyrY: {:.2f}\nGyrZ: {:.2f}'.format(AccX_val, AccY_val, AccZ_val, GyrX_val, GyrY_val, GyrZ_val))
        plt.pause(.01)