import cv2
import cv2.aruco as aruco
import numpy as np
import socket
import pickle

# Set up Aruco dictionary.
aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_50)

# Set the parameters for Aruco marker detection.
params = aruco.DetectorParameters_create()

# Initialize the camera capture.
cap = cv2.VideoCapture(0)


# Create a dictionary to store the distance and coordinates for each marker.
marker_data = {}


# Create a socket object which we will use to send the dictionary data w/ UDP protocol
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Define the IP address and port of the receiver (for the socket shit)
IP = "127.0.0.1"
PORT = 12347

# Set the camera calibration parameters.
# These parameters should be obtained through calibration of the camera.
# See the OpenCV documentation for details:
# https://docs.opencv.org/3.4/dc/dbb/tutorial_py_calibration.html
camera_matrix = np.array([[1.3, 0, 0], [0, 1.3, 0], [0, 0, 1]])
dist_coeffs = np.array([0, 0, 0, 0, 0])

while True:
    # Capture frame-by-frame.
    ret, frame = cap.read()

    # Detect Aruco markers.
    corners, ids, rejectedImgPoints = aruco.detectMarkers(frame, aruco_dict, parameters=params)

    # Check if any markers were detected.
    if ids is not None and len(ids) > 0:
        # Calculate the 3D position of the Aruco marker relative to the camera.
        # The distance is calculated as the Euclidean norm of the 3D position vector.
        rvec, tvec, _ = aruco.estimatePoseSingleMarkers(corners, 0.05, camera_matrix, dist_coeffs)
        distance = np.linalg.norm(tvec)


        # Get the x, y, and z coordinates of the marker.
        x, y, z = tvec[0][0]

        # Update the dictionary with the marker information.
        for id in ids:
            marker_data[int(id)] = (distance, (x, y, z))
            serialized_data = pickle.dumps(marker_data) #serialize our marker data so we can send with a socket
            s.sendto(serialized_data, (IP, PORT)) #send the serialized data to the defined IP & PORT(Mike's shit)
            print(id) #Occaisionally, marker # 34 or 37 will be picked up by the listener script, but it isn't printed out here...

        # Draw the Aruco marker on the frame.
        frame = aruco.drawDetectedMarkers(frame, corners, ids)

        # Display the distance on the frame.
        font = cv2.FONT_HERSHEY_SIMPLEX
        cv2.putText(frame, f'distance: {distance:.2f}', (0, 50), font, 1, (0, 255, 0), 2, cv2.LINE_AA)
        #Display the x,y & z coordinates
        cv2.putText(frame, f'coordinates: ({x:.2f}, {y:.2f}, {z:.2f})', (0, 100), font, 1, (0, 255, 0), 2, cv2.LINE_AA)


    # Display the resulting frame.
    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything is done, release the capture.
cap.release()
cv2.destroyAllWindows()
#Close socket w/UDP shit
s.close()
