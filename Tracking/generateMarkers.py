import cv2
import cv2.aruco as aruco

# Set the marker size (in pixels)
marker_size = 200

# Create a dictionary of aruco markers
aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_50)

# Generate and save each marker
for i in range(50):
    # Generate the marker image
    marker_img = aruco.drawMarker(aruco_dict, i, marker_size)

    # Save the marker image to a PNG file
    filename = "marker_{}.png".format(i)
    cv2.imwrite(filename, marker_img)
