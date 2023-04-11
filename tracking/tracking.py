import socket  # Import the socket library
import sys

import cv2
import numpy as np

# Constants
ARUCO_DICT = cv2.aruco.Dictionary_get(cv2.aruco.DICT_4X4_50)
ARUCO_PARAMETERS = cv2.aruco.DetectorParameters_create()
CAMERA_MATRIX = np.load('camera_matrix.npy')
DIST_COEFFS = np.load('dist_coeffs.npy')
PERPENDICULAR_DISTANCE = 0.1  # 10 cm
PARALLEL_DISTANCE = 0.3  # also 10 cm for now

# UDP socket configuration
IP = "127.0.0.1"  # Replace with the IP address you want to send the data to
PORT = 16000
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)  # Create a UDP socket
marker_data = []

previous_smoothed_imaginary_point = np.zeros((3, 1))


def main():
    cap = cv2.VideoCapture(0)

    tracked_id = None
    frame_without_marker = 0

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                break

            gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            corners, ids, _ = cv2.aruco.detectMarkers(gray, ARUCO_DICT, parameters=ARUCO_PARAMETERS)

            if ids is not None and (tracked_id is None or tracked_id in ids):
                rvecs, tvecs, _ = cv2.aruco.estimatePoseSingleMarkers(corners, 1, CAMERA_MATRIX, DIST_COEFFS)
                for i, id in enumerate(ids):
                    if tracked_id is None:
                        tracked_id = id
                    if id == tracked_id:
                        frame_without_marker = 0
                        rvec, tvec = rvecs[i], tvecs[i]
                        if id == 0:
                            imaginary_point = get_imaginary_point(PERPENDICULAR_DISTANCE, rvec, tvec)
                        else:
                            imaginary_point = get_imaginary_point_minus_y(PARALLEL_DISTANCE, rvec, tvec)

                        draw_imaginary_point(frame, imaginary_point)

                        x, y, z = imaginary_point[0][0], imaginary_point[1][0], imaginary_point[2][0]

                        # Filtering jitter
                        global previous_smoothed_imaginary_point
                        x_filtered = exponential_moving_average(x, previous_smoothed_imaginary_point[0][0], 0.1)
                        y_filtered = exponential_moving_average(y, previous_smoothed_imaginary_point[1][0], 0.1)
                        z_filtered = exponential_moving_average(z, previous_smoothed_imaginary_point[2][0], 0.1)
                        # If this causes issues remove the above 4 lines and send x,y and z normally in marker data

                        previous_smoothed_imaginary_point = np.array([[x_filtered], [y_filtered], [z_filtered]])

                        R, _ = cv2.Rodrigues(rvec)
                        euler_angles = rotation_matrix_to_euler_angles(R)
                        pitch, yaw, roll = euler_angles

                        # Send the data through the UDP socket
                        marker_data = f"{x_filtered} {y_filtered} {z_filtered} {pitch} {yaw} {roll}"
                        print(marker_data)
                        # Send data over UDP socket
                        serialized_data = marker_data.encode()
                        s.sendto(serialized_data, (IP, PORT))
                        break
            else:
                frame_without_marker += 1
                if frame_without_marker > 10:
                    tracked_id = None

            cv2.imshow('frame', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    except:
        # When everything is done, release the capture
        cap.release()
        cv2.destroyAllWindows()
        # Close socket
        s.close()
        print("Closing tracking system gracefully.")
        sys.exit()


def exponential_moving_average(value, previous_value, alpha):
    return alpha * value + (1 - alpha) * previous_value


def get_imaginary_point(distance, rvec, tvec):
    R = cv2.Rodrigues(rvec)[0]
    T = tvec.reshape(3, 1)
    P = np.float32([0, 0, -distance, 1]).reshape(4, 1)
    imaginary_point = np.matmul(R, P[:3]) + T
    return imaginary_point


def get_imaginary_point_minus_y(distance, rvec, tvec):
    R = cv2.Rodrigues(rvec)[0]
    T = tvec.reshape(3, 1)
    P = np.float32([0, -distance, 0, 1]).reshape(4, 1)  # Move the point in the -y direction
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
