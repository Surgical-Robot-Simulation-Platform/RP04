import cv2

def test_camera(index):
    cap = cv2.VideoCapture(index)
    if not cap.isOpened():
        print(f"Camera with index {index} not found.")
        cap.release()
        return False

    ret, frame = cap.read()
    if not ret:
        print(f"Camera with index {index} found, but unable to read frame.")
        cap.release()
        return False

    cap.release()
    return True

def find_camera_index():
    index = 3
    while True:
        if test_camera(index):
            print(f"Camera found at index {index}")
            return index
        index += 1
        if index > 10:
            print("No camera found up to index 10. Exiting.")
            return None

camera_index = find_camera_index()
