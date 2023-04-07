import glob

import cv2
import numpy as np

# Prepare obj points for a 9x6 chessboard pattern - so we'll actually be using a physical 10x7 printed one
objp = np.zeros((6 * 9, 3), np.float32)
objp[:, :2] = np.mgrid[0:9, 0:6].T.reshape(-1, 2)

# Arrays to store object points and image points from all the images
objpoints = []  # 3D = real world space
imgpoints = []  # 2D = image plane

# get all the calibration pics I took
images = glob.glob('calibration_images\\*.png')
if len(images) == 0:
    images = glob.glob('calibration_images\\*.jpg')
for fname in images:
    print(fname)
    img = cv2.imread(fname)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    # Find corners on chessboard
    ret, corners = cv2.findChessboardCorners(gray, (9, 6), None)

    # If found, add object points, image points (after refining them)
    if ret == True:
        print(f"Chessboard corners detected in {fname}")
        objpoints.append(objp)

        corners2 = cv2.cornerSubPix(gray, corners, (11, 11), (-1, -1),
                                    (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001))
        imgpoints.append(corners2)
    else:
        print(f"Failed to detect chessboard corners in {fname}")

# Finally Calibrating camera using shit found above
ret, camera_matrix, dist_coeffs, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None)

print("Camera Matrix: \n", camera_matrix)
print("Distortion Coefficients: \n", dist_coeffs)
np.save('camera_matrix.npy', camera_matrix)
np.save('dist_coeffs.npy', dist_coeffs)
