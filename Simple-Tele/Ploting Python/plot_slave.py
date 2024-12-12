import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime
import sys
import re

def plot_saved_data(csv_filename):
    try:
        data = pd.read_csv(csv_filename)
    except FileNotFoundError:
        print(f"Error: Could not find file {csv_filename}")
        return
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    # Angle tracking plot
    fig1, ax1 = plt.subplots(figsize=(10, 4))
    ax1.plot(data['time'], data['desired_angle'], 'b-', label='Desired Angle')
    ax1.plot(data['time'], data['current_angle'], 'r--', label='Current Angle')
    title1 = 'Angle Tracking'
    ax1.set_title(title1)
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Angle (rad)')
    ax1.legend()
    ax1.grid(True)
    plt.tight_layout()
    filename1 = re.sub(r'[^a-zA-Z0-9]', '_', title1.lower())
    plt.savefig(f'{filename1}_{timestamp}.png')
    
    # Velocity profile plot
    fig2, ax2 = plt.subplots(figsize=(10, 4))
    ax2.plot(data['time'], data['velocity'], 'g-', label='Velocity')
    title2 = 'Velocity Profile'
    ax2.set_title(title2)
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('Velocity (rad/s)')
    ax2.legend()
    ax2.grid(True)
    plt.tight_layout()
    filename2 = re.sub(r'[^a-zA-Z0-9]', '_', title2.lower())
    plt.savefig(f'{filename2}_{timestamp}.png')
    
    # Position error plot
    fig3, ax3 = plt.subplots(figsize=(10, 4))
    ax3.plot(data['time'], data['error'], 'm-', label='Position Error')
    title3 = 'Position Error'
    ax3.set_title(title3)
    ax3.set_xlabel('Time (s)')
    ax3.set_ylabel('Error (rad)')
    ax3.legend()
    ax3.grid(True)
    plt.tight_layout()
    filename3 = re.sub(r'[^a-zA-Z0-9]', '_', title3.lower())
    plt.savefig(f'{filename3}_{timestamp}.png')
    
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        csv_filename = sys.argv[1]
    else:
        csv_filename = input("Enter the CSV filename to plot: ")
    
    plot_saved_data(csv_filename)