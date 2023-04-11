import socket
import sys

import cv2
import numpy as np

# Constants
ARUCO_DICT = cv2.aruco.Dictionary_get(cv2.aruco.DICT_4X4_50)
ARUCO_PARAMETERS = cv2.aruco.DetectorParameters_create()
CAMERA_MATRIX = np.load('camera_matrix.npy')
DIST_COEFFS = np.load('dist_coeffs.npy')
PERPENDICULAR_DISTANCE = 1  # 10 cm

# UDP socket configuration
IP = "127.0.0.1"  # Replace with the IP address you want to send the data to
PORT = 12347
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Create a UDP socket
marker_data = []


def main():
    try:
        cap = cv2.VideoCapture(0)

        while True:
            ret, frame = cap.read()
            if not ret:
                break

            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            corners, ids, _ = cv2.aruco.detectMarkers(gray, ARUCO_DICT, parameters=ARUCO_PARAMETERS)

            if ids is not None and 0 in ids:
                rvecs, tvecs, _ = cv2.aruco.estimatePoseSingleMarkers(corners, 1, CAMERA_MATRIX, DIST_COEFFS)
                for i, id in enumerate(ids):
                    if id == 0:
                        rvec, tvec = rvecs[i], tvecs[i]
                        imaginary_point = get_imaginary_point(PERPENDICULAR_DISTANCE, rvec, tvec)
                        draw_imaginary_point(frame, imaginary_point)

                        x, y, z = imaginary_point[0][0], imaginary_point[1][0], imaginary_point[2][0]

                        R, _ = cv2.Rodrigues(rvec)
                        euler_angles = rotation_matrix_to_euler_angles(R)
                        pitch, yaw, roll = euler_angles

                        # Send the data through the UDP socket
                        marker_data = f"{x} {y} {z} {pitch} {yaw} {roll}"
                        print(marker_data)
                        # Send data over UDP socket
                        serialized_data = marker_data.encode()
                        s.sendto(serialized_data, (IP, PORT))
                        break

            cv2.imshow('frame', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                raise KeyboardInterrupt
    except KeyboardInterrupt:
        # When everything is done, release the capture
        cap.release()
        cv2.destroyAllWindows()
        # Close socket w/UDP shit
        s.close()
        print("Closing tracking system gracefully.")
        sys.exit()


def get_imaginary_point(distance, rvec, tvec):
    R = cv2.Rodrigues(rvec)[0]
    T = tvec.reshape(3, 1)
    P = np.float32([0, 0, -distance, 1]).reshape(4, 1)
    imaginary_point = np.matmul(R, P[:3]) + T
    return imaginary_point


# this thing might fuck me
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


def draw_imaginary_point(frame, point):
    point_2d, _ = cv2.projectPoints(point, np.zeros(3), np.zeros(3), CAMERA_MATRIX, DIST_COEFFS)
    x, y = int(point_2d[0][0][0]), int(point_2d[0][0][1])
    cv2.circle(frame, (x, y), 5, (0, 0, 255), -1)


if __name__ == "__main__":
    main()
