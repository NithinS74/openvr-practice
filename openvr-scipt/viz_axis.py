import openvr
import numpy as np
from vispy import app, scene
from vispy.visuals import transforms

# --- 1. VR ---
def get_pose(vr, idx):
    pose = vr.getDeviceToAbsoluteTrackingPose(openvr.TrackingUniverseStanding, 0, openvr.k_unMaxTrackedDeviceCount)
    if pose[idx].bPoseIsValid:
        m = pose[idx].mDeviceToAbsoluteTracking
        # 4x4 matrix
        mat = np.array([
            [m[0][0], m[0][1], m[0][2], m[0][3]],
            [m[1][0], m[1][1], m[1][2], m[1][3]],
            [m[2][0], m[2][1], m[2][2], m[2][3]],
            [0, 0, 0, 1]
        ])
        return mat
    return None

try:
    vr = openvr.init(openvr.VRApplication_Background)
except:
    print("SteamVR invalid.")
    exit(1)

t_id = -1
for i in range(openvr.k_unMaxTrackedDeviceCount):
    cls = vr.getTrackedDeviceClass(i)
    if cls in [openvr.TrackedDeviceClass_GenericTracker, openvr.TrackedDeviceClass_Controller]:
        t_id = i
        break

if t_id == -1:
    print("No dev found.")

# --- 2. Viz ---
canv = scene.SceneCanvas(keys='interactive', show=True, title='VR Axis Center')
view = canv.central_widget.add_view()
view.camera = 'turntable'
view.camera.distance = 1.5
view.camera.center = (0, 0, 0) # Lock camera to origin

# Axis
ax = scene.visuals.XYZAxis(parent=view.scene, width=5)

# --- 3. Loop ---
def update(ev):
    if t_id == -1: return

    m = get_pose(vr, t_id)
    
    if m is not None:
        # FORCE POSITION TO ZERO
        # This keeps the axis in the middle of the screen, ignoring room position
        m[0, 3] = 0
        m[1, 3] = 0
        m[2, 3] = 0

        tr = transforms.MatrixTransform()
        tr.matrix = m.T 
        ax.transform = tr
        
    canv.update()

timer = app.Timer(interval=1/60.0, connect=update, start=True)

if __name__ == '__main__':
    app.run()