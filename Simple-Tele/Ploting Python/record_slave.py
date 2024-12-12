import serial
import csv
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from datetime import datetime
import numpy as np
from collections import deque

class DataPlotter:
    def __init__(self, max_points=500):
        # Data storage
        self.times = deque(maxlen=max_points)
        self.desired_angles = deque(maxlen=max_points)
        self.current_angles = deque(maxlen=max_points)
        self.velocities = deque(maxlen=max_points)
        self.errors = deque(maxlen=max_points)
        
        # Setup plot
        self.fig, (self.ax1, self.ax2, self.ax3) = plt.subplots(3, 1, figsize=(10, 12))
        
        # Initialize lines
        self.line_desired, = self.ax1.plot([], [], 'b-', label='Desired Angle')
        self.line_current, = self.ax1.plot([], [], 'r-', label='Current Angle')
        self.line_velocity, = self.ax2.plot([], [], 'g-', label='Velocity')
        self.line_error, = self.ax3.plot([], [], 'm-', label='Position Error')
        
        # Setup axes
        self.setup_axes()
        
    def setup_axes(self):
        self.ax1.set_title('Angle Tracking')
        self.ax1.set_xlabel('Time (s)')
        self.ax1.set_ylabel('Angle (rad)')
        self.ax1.legend()
        
        self.ax2.set_title('Velocity Profile')
        self.ax2.set_xlabel('Time (s)')
        self.ax2.set_ylabel('Velocity (rad/s)')
        self.ax2.legend()
        
        self.ax3.set_title('Position Error')
        self.ax3.set_xlabel('Time (s)')
        self.ax3.set_ylabel('Error (rad)')
        self.ax3.legend()
        
        plt.tight_layout()
        
    def update_plot(self, frame):
        # Update line data
        self.line_desired.set_data(list(self.times), list(self.desired_angles))
        self.line_current.set_data(list(self.times), list(self.current_angles))
        self.line_velocity.set_data(list(self.times), list(self.velocities))
        self.line_error.set_data(list(self.times), list(self.errors))
        
        # Adjust axes limits
        for ax in [self.ax1, self.ax2, self.ax3]:
            ax.relim()
            ax.autoscale_view()
        
        return self.line_desired, self.line_current, self.line_velocity, self.line_error

def main():
    # Setup serial connection
    ser = serial.Serial('COM4', 115200, timeout=1)
    time.sleep(2)  # Wait for connection to stabilize
    
    # Create data plotter
    plotter = DataPlotter()
    
    # Setup CSV file
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f'slave_data_{timestamp}.csv'
    
    with open(filename, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(['time', 'desired_angle', 'current_angle', 'velocity', 'error'])
        
        # Setup animation
        ani = animation.FuncAnimation(plotter.fig, plotter.update_plot, interval=100)
        plt.show(block=False)
        
        start_time = time.time()
        
        try:
            while True:
                if ser.in_waiting:
                    line = ser.readline().decode().strip()
                    try:
                        current_time = time.time() - start_time
                        desired_angle = float(line.split(',')[1])
                        current_angle = float(line.split(',')[2])
                        velocity = float(line.split(',')[3])
                        error = float(line.split(',')[4])
                        
                        # Update data
                        plotter.times.append(current_time)
                        plotter.desired_angles.append(desired_angle)
                        plotter.current_angles.append(current_angle)
                        plotter.velocities.append(velocity)
                        plotter.errors.append(error)
                        
                        # Write to CSV
                        csvwriter.writerow([current_time, desired_angle, current_angle, 
                                         velocity, error])
                        csvfile.flush()
                        
                    except (ValueError, IndexError) as e:
                        print(f"Error parsing line: {line}")
                        continue
                        
        except KeyboardInterrupt:
            print("\nStopping data collection...")
            ser.close()
            plt.close()
            
            # Create final static plots
            create_static_plots(filename)

def create_static_plots(filename):
    data = np.genfromtxt(filename, delimiter=',', skip_header=1)
    
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(10, 12))
    
    # Angle tracking
    ax1.plot(data[:,0], data[:,1], 'b', label='Desired Angle')
    ax1.plot(data[:,0], data[:,2], 'r--', label='Current Angle')
    ax1.set_title('Angle Tracking')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Angle (rad)')
    ax1.legend()
    
    # Velocity profile
    ax2.plot(data[:,0], data[:,3], 'g-', label='Velocity')
    ax2.set_title('Velocity Profile')
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('Velocity (rad/s)')
    ax2.legend()
    
    # Position error
    ax3.plot(data[:,0], data[:,4], 'm-', label='Position Error')
    ax3.set_title('Position Error')
    ax3.set_xlabel('Time (s)')
    ax3.set_ylabel('Error (rad)')
    ax3.legend()
    
    plt.tight_layout()
    plt.savefig(f'analysis_plots_{datetime.now().strftime("%Y%m%d_%H%M%S")}.png')
    plt.show()

if __name__ == "__main__":
    main()