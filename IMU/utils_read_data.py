import socket
import json
import numpy as np


class IKEF:
    
    def __init__(self, parameter_class=None):
        self.g = None
        self.omega = None
        self.a = None 
        
    class Parameters:
        g = np.array([0, 0, -9.80665])
        
        
    def receive_udp_data(port):
        # Create a UDP socket
        udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        udp_socket.bind(('localhost', port))
        print(f"Listening for UDP data on port {port}...")
        while True:
            data, addr = udp_socket.recvfrom(1024)
            received_data = data.decode('utf-8')
            try:
                json_data = json.loads(received_data)
                print(json_data["acceleration"]["x"])
            except json.JSONDecodeError as e:
                print(f"Error decoding JSON: {e}")
                continue


udp_port = 5500
receive_udp_data(udp_port)

import numpy as np

class ExtendedKalmanFilter:
    def __init__(self, initial_state, initial_covariance, process_noise_covariance, measurement_noise_covariance):
        self.state = initial_state
        self.covariance = initial_covariance
        self.Q = process_noise_covariance
        self.R = measurement_noise_covariance

    def predict(self, angular_velocity, acceleration, dt):
        # Prediction step
        F = self.compute_state_transition_matrix(angular_velocity, dt)
        self.state = self.state_prediction(self.state, angular_velocity, acceleration, dt)
        self.covariance = self.predict_covariance(F, self.covariance, self.Q)
        
    def update(self, rotation_matrix):
        # Update step
        H = self.compute_observation_matrix()
        innovation, innovation_covariance, kalman_gain = self.compute_kalman_gain(H, self.covariance, self.R, rotation_matrix)
        self.state = self.correct_state(self.state, kalman_gain, innovation)
        self.covariance = self.correct_covariance(kalman_gain, self.covariance)

    def state_prediction(self, state, angular_velocity, acceleration, dt):
        # Implement your state prediction model here
        # For example, use the IMU measurements to predict the next state
        # state = f(state, angular_velocity, acceleration, dt)
        return state

    def compute_state_transition_matrix(self, angular_velocity, dt):
        # Implement the computation of the state transition matrix
        # F = df/dx, where x is the state vector
        # F = df/dx = identity_matrix + df/dw * dw/dx * dt + df/da * da/dx * dt
        return np.eye(len(self.state))

    def predict_covariance(self, F, covariance, Q):
        # Implement the covariance prediction using the state transition matrix
        # P = F * P * F^T + Q
        return np.dot(np.dot(F, covariance), F.T) + Q

    def compute_observation_matrix(self):
        # Implement the computation of the observation matrix
        # H = dh/dx, where x is the state vector
        return np.eye(len(self.state))

    def compute_kalman_gain(self, H, covariance, R, rotation_matrix):
        # Implement the computation of the Kalman gain
        # K = P * H^T * (H * P * H^T + R)^-1
        innovation_covariance = np.dot(np.dot(H, covariance), H.T) + R
        kalman_gain = np.dot(np.dot(covariance, H.T), np.linalg.inv(innovation_covariance))
        innovation = rotation_matrix - np.dot(H, self.state)
        return innovation, innovation_covariance, kalman_gain

    def correct_state(self, state, kalman_gain, innovation):
        # Implement the state correction step
        # x = x + K * (z - H * x)
        return state + np.dot(kalman_gain, innovation)

    def correct_covariance(self, kalman_gain, covariance):
        # Implement the covariance correction step
        # P = (I - K * H) * P
        return np.dot((np.eye(len(self.state)) - np.dot(kalman_gain, H)), covariance)

# Example usage:
initial_state = np.array([0, 0, 0, 0, 0, 0])  # Example initial state for 6DOF (orientation and velocity)
initial_covariance = np.eye(6)  # Example initial covariance matrix
process_noise_covariance = np.eye(6) * 1e-6  # Example process noise covariance matrix
measurement_noise_covariance = np.eye(9) * 1e-3  # Example measurement noise covariance matrix

ekf = ExtendedKalmanFilter(initial_state, initial_covariance, process_noise_covariance, measurement_noise_covariance)

# Measurements from IMU (angular_velocity, acceleration, rotation_matrix)
angular_velocity = np.array([0.1, 0.2, 0.3])
acceleration = np.array([0.0, 0.0, 9.8])
rotation_matrix = np.eye(3)  # Example rotation matrix, replace with actual measurement

# Prediction step
ekf.predict(angular_velocity, acceleration, dt=0.01)

# Update step
ekf.update(rotation_matrix)
