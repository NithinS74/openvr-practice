import openvr
import numpy as np
from vispy import app, scene

# --- 1. OpenVR Setup ---
def get_pose(vr, idx):
    poses = vr.getDeviceToAbsoluteTrackingPose(openvr.TrackingUniverseStanding, 0, openvr.k_unMaxTrackedDeviceCount)
    if poses[idx].bPoseIsValid:
        m = poses[idx].mDeviceToAbsoluteTracking
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
    # Fallback if no headset, just to let code run (will fail at loop)
    print("SteamVR Init Error")
    exit(1)

t_idx = -1
for i in range(openvr.k_unMaxTrackedDeviceCount):
    if vr.getTrackedDeviceClass(i) == openvr.TrackedDeviceClass_GenericTracker:
        t_idx = i
        print(f"Tracker found: {i}")
        break

# --- 2. VisPy Scene ---
canvas = scene.SceneCanvas(keys='interactive', show=True, title='VR Stick Figure')
view = canvas.central_widget.add_view()
view.camera = 'turntable'
view.camera.distance = 3
view.camera.center = (0, 1, 0)

# Points
head = np.array([0, 1.8, 0])
neck = np.array([0, 1.5, 0])
hip  = np.array([0, 0.9, 0])
r_sh = np.array([0.2, 1.4, 0]) # Right Shoulder
l_sh = np.array([-0.2, 1.4, 0])
l_el = np.array([-0.25, 1.1, 0])
l_ha = np.array([-0.25, 0.8, 0])
r_hip= np.array([0.15, 0.9, 0])
l_hip= np.array([-0.15, 0.9, 0])
r_kn = np.array([0.15, 0.5, 0])
l_kn = np.array([-0.15, 0.5, 0])
r_ft = np.array([0.15, 0.0, 0])
l_ft = np.array([-0.15, 0.0, 0])

static_seg = [
    head, neck, neck, hip,
    neck, r_sh, neck, l_sh,
    l_sh, l_el, l_el, l_ha,
    hip, r_hip, hip, l_hip,
    r_hip, r_kn, l_hip, l_kn,
    r_kn, r_ft, l_kn, l_ft
]

skel_vis = scene.visuals.Line(width=5, color='white', connect='segments', parent=view.scene)
joint_vis = scene.visuals.Markers(parent=view.scene, face_color='red', size=10)

# --- 3. Calibration Vars ---
calibrated = False
offset = np.array([0., 0., 0.])

# Ideal starting position for the hand (relative to shoulder)
# This places the hand slightly down and forward from the shoulder
default_hand_rel = np.array([0.1, -0.4, 0.2]) 

def update(ev):
    global calibrated, offset, t_idx
    
    # Retry finding tracker if not found initially
    if t_idx == -1: return 

    mat = get_pose(vr, t_idx)
    
    if mat is not None:
        raw_pos = mat[:3, 3]
        rot = mat[:3, :3]
        
        # --- CALIBRATION STEP ---
        if not calibrated:
            # Calculate where the hand SHOULD be in the virtual world
            target_pos = r_sh + default_hand_rel
            # Calculate difference between Real Tracker and Virtual Target
            offset = target_pos - raw_pos
            calibrated = True
            print("Calibrated! Hand snapped to body.")

        # Apply offset to raw tracker data
        wrist_pos = raw_pos + offset

        # Calculate fingertips
        fwd = np.array([0, 0, -1]) 
        hand_vec = rot @ fwd
        tip_pos = wrist_pos + (hand_vec * 0.15) 

        # Update visual
        pts = static_seg.copy()
        
        # Shoulder -> Wrist
        pts.append(r_sh)
        pts.append(wrist_pos)
        
        # Wrist -> Tip
        pts.append(wrist_pos)
        pts.append(tip_pos)
        
        skel_vis.set_data(pos=np.array(pts))
        joint_vis.set_data(pos=np.array([wrist_pos]))
        
    canvas.update()

timer = app.Timer(interval=1/60.0, connect=update, start=True)

if __name__ == '__main__':
    app.run()