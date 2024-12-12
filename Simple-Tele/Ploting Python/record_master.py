import serial
import csv
import time
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from datetime import datetime
import numpy as np
from collections import deque

class MasterPlotter:
    def __init__(self, max_points=500):
        # Data storage
        self.times = deque(maxlen=max_points)
        self.positions = deque(maxlen=max_points)
        self.resistances = deque(maxlen=max_points)
        
        # Setup plot
        self.fig, (self.ax1, self.ax2) = plt.subplots(2, 1, figsize=(10, 8))
        
        # Initialize lines
        self.line_position, = self.ax1.plot([], [], 'b-', label='Position')
        self.line_resistance, = self.ax2.plot([], [], 'r-', label='Resistance')
        
        # Setup axes
        self.setup_axes()
        
    def setup_axes(self):
        self.ax1.set_title('Position vs Time')
        self.ax1.set_xlabel('Time (s)')
        self.ax1.set_ylabel('Position (rad)')
        self.ax1.legend()
        
        self.ax2.set_title('Resistance vs Time')
        self.ax2.set_xlabel('Time (s)')
        self.ax2.set_ylabel('Resistance')
        self.ax2.legend()
        
        plt.tight_layout()
        
    def update_plot(self, frame):
        # Update line data
        self.line_position.set_data(list(self.times), list(self.positions))
        self.line_resistance.set_data(list(self.times), list(self.resistances))
        
        # Adjust axes limits
        for ax in [self.ax1, self.ax2]:
            ax.relim()
            ax.autoscale_view()
        
        return self.line_position, self.line_resistance

def main():
    ser = serial.Serial('COM3', 115200, timeout=1)
    time.sleep(2)
    
    plotter = MasterPlotter()
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f'master_data_{timestamp}.csv'
    
    with open(filename, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        # Update headers to match actual data
        csvwriter.writerow(['time', 'position', 'velocity', 'resistance'])
        
        ani = animation.FuncAnimation(plotter.fig, plotter.update_plot, interval=100)
        plt.show(block=False)
        
        start_time = time.time()
        
        try:
            while True:
                if ser.in_waiting:
                    line = ser.readline().decode().strip()
                    try:
                        current_time = time.time() - start_time
                        values = line.split(',')
                        # Parse 4 values instead of 6
                        position = float(values[0])
                        velocity = float(values[1])
                        resistance = float(values[3])  # Resistance is the 4th value
                        
                        # Update plot data
                        plotter.times.append(current_time)
                        plotter.positions.append(position)
                        plotter.resistances.append(resistance)
                        
                        # Write to CSV
                        csvwriter.writerow([current_time, position, velocity, resistance])
                        csvfile.flush()
                        
                    except (ValueError, IndexError) as e:
                        print(f"Error parsing line: {line}")
                        continue
                        
        except KeyboardInterrupt:
            print("\nStopping data collection...")
            ser.close()
            plt.close()
            create_static_plots(filename)

def create_static_plots(filename):
    data = np.genfromtxt(filename, delimiter=',', skip_header=1)
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
    
    # Position plot - column 1
    ax1.plot(data[:,0], data[:,1], 'b-', label='Position')
    ax1.set_title('Position vs Time')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Position (rad)')
    ax1.legend()
    
    # Resistance plot - column 3
    ax2.plot(data[:,0], data[:,3], 'r-', label='Resistance')
    ax2.set_title('Resistance vs Time')
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('Resistance')
    ax2.legend()
    
    plt.tight_layout()
    plt.savefig(f'master_analysis_plots_{datetime.now().strftime("%Y%m%d_%H%M%S")}.png')
    plt.show()

if __name__ == "__main__":
    main()