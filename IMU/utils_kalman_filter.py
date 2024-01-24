import numpy as np

def skew_symmetric_matrix(vector):
    return np.array([[0, -vector[2], vector[1]],
                     [vector[2], 0, -vector[0]],
                     [-vector[1], vector[0], 0]])

def rotation_update(R_prev, omega, dt):
    skew_matrix = skew_symmetric_matrix(omega)
    exp_skew_matrix = np.eye(3) + skew_matrix * dt
    R_next = np.dot(R_prev, exp_skew_matrix)
    return R_next

def velocity_update(R, v_prev, a, g, dt):
    gravity = np.array([0, 0, -g])
    v_next = v_prev + np.dot(R, a) * dt + gravity * dt
    return v_next

def position_update(v, p_prev, dt):
    p_next = p_prev + v * dt
    return p_next

# Example Usage:
# Initialize variables
RIMU = np.eye(3)  # Initial rotation matrix
vIMU = np.array([0, 0, 0])  # Initial velocity vector
pIMU = np.array([0, 0, 0])  # Initial position vector
omega_n = np.array([0.1, 0.2, 0.3])  # Angular velocity
a_n = np.array([0.1, 0.2, 9.8])  # Acceleration
g = 9.8  # Gravity
dt = 0.01  # Time step

# Rotation Update
RIMU_next = rotation_update(RIMU, omega_n, dt)

# Velocity Update
vIMU_next = velocity_update(RIMU, vIMU, a_n, g, dt)

# Position Update
pIMU_next = position_update(vIMU, pIMU, dt)

# Print Results
print("Rotation Matrix (RIMU):")
print(RIMU_next)

print("\nVelocity Vector (vIMU):")
print(vIMU_next)

print("\nPosition Vector (pIMU):")
print(pIMU_next)
