import cv2
import cv2.aruco as aruco
import numpy as np
import socket
import pickle

def rotation_matrix_to_euler_angles(R):
    # FInding yaw, pitch, and roll from the rotation matrix R
    sy = np.sqrt(R[0, 0] * R[0, 0] + R[1, 0] * R[1, 0])
    singular = sy < 1e-6
    if not singular:
        x = np.arctan2(R[2, 1], R[2, 2])
        y = np.arctan2(-R[2, 0], sy)
        z = np.arctan2(R[1, 0], R[0, 0])
    else:
        x = np.arctan2(-R[1, 2], R[1, 1])
        y = np.arctan2(-R[2, 0], sy)
        z = 0
    return np.rad2deg(np.array([x, y, z]))

#Set up Aruco dictionary.
aruco_dict = aruco.Dictionary_get(aruco.DICT_4X4_50)

#Set the parameters for Aruco marker detection.
params = aruco.DetectorParameters_create()

# Initialize the camera capture.
cap = cv2.VideoCapture(0)

# Create a dictionary to store the distance and coordinates for each marker.
marker_data = {}

# Create a socket object which we will use to send the dictionary data w/ UDP protocol
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Define the IP address and port of the receiver (for the socket shit)
IP = "127.0.0.1"
PORT = 16000

# Set the camera calibration parameters.
# Loading the camera matrix and distortion coefficients
camera_matrix = np.load('camera_matrix.npy')
dist_coeffs = np.load('dist_coeffs.npy')

tracked_id = None
frames_without_marker = 0

while True:
    # Capture frame-by-frame.
    ret, frame = cap.read()

    # Detect Aruco markers.
    corners, ids, rejectedImgPoints = aruco.detectMarkers(frame, aruco_dict, parameters=params)

    if tracked_id is not None and (ids is None or tracked_id not in ids):
        frames_without_marker += 1
    else:
        frames_without_marker = 0

    if frames_without_marker >= 10:
        tracked_id = None

    if ids is not None and (tracked_id is None or tracked_id in ids):
        if tracked_id is None:
            tracked_id = ids[0]

        tracked_index = np.where(ids == tracked_id)[0][0]
        corner = corners[tracked_index]

        rvec, tvec, _ = aruco.estimatePoseSingleMarkers(corner, 0.05, camera_matrix, dist_coeffs)
        distance = np.linalg.norm(tvec)
        R, _ = cv2.Rodrigues(rvec)
        euler_angles = rotation_matrix_to_euler_angles(R)
        pitch, yaw, roll = euler_angles
        x, y, z = tvec[0][0]

        marker_data = f"{x} {y} {z} {pitch} {yaw} {roll}"
        serialized_data = marker_data.encode()
        s.sendto(serialized_data, (IP, PORT))

        frame = aruco.drawDetectedMarkers(frame, [corner], np.array([tracked_id]))
        font = cv2.FONT_HERSHEY_SIMPLEX
        x_offset = int(corner[0][0][0])
        y_offset = int(corner[0][0][1])
        cv2.putText(frame, f'distance: {distance:.2f}', (x_offset, y_offset - 40), font, 0.5, (0, 255, 0), 1, cv2.LINE_AA)
        cv2.putText(frame, f'coordinates: ({x:.2f}, {y:.2f}, {z:.2f})', (x_offset, y_offset - 20), font, 0.5, (0, 255, 0), 1, cv2.LINE_AA)
        cv2.putText(frame, f'pitch: {pitch:.2f}, yaw: {yaw:.2f}, roll: {roll:.2f}', (x_offset, y_offset - 60), font, 0.5, (255, 255, 255), 1, cv2.LINE_AA)

    # Display the resulting frame.
    cv2.imshow('frame', frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break
