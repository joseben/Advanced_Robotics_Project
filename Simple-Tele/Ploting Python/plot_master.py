import pandas as pd
import matplotlib.pyplot as plt
from datetime import datetime
import sys
import re

def plot_master_data(csv_filename):
    try:
        data = pd.read_csv(csv_filename)
    except FileNotFoundError:
        print(f"Error: Could not find file {csv_filename}")
        return
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    # Position plot
    fig1, ax1 = plt.subplots(figsize=(10, 4))
    ax1.plot(data['time'], data['position'], 'b-', label='Position')
    title1 = 'Position vs Time'
    ax1.set_title(title1)
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Position (rad)')
    ax1.legend()
    ax1.grid(True)
    plt.tight_layout()
    # Clean filename
    filename1 = re.sub(r'[^a-zA-Z0-9]', '_', title1.lower())
    plt.savefig(f'{filename1}_{timestamp}.png')
    
    # Resistance plot
    fig2, ax2 = plt.subplots(figsize=(10, 4))
    ax2.plot(data['time'], data['resistance'], 'r-', label='Resistance')
    title2 = 'Resistance vs Time'
    ax2.set_title(title2)
    ax2.set_xlabel('Time (s)')
    ax2.set_ylabel('Resistance')
    ax2.legend()
    ax2.grid(True)
    plt.tight_layout()
    # Clean filename
    filename2 = re.sub(r'[^a-zA-Z0-9]', '_', title2.lower())
    plt.savefig(f'{filename2}_{timestamp}.png')
    
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        csv_filename = sys.argv[1]
    else:
        csv_filename = input("Enter the master CSV filename to plot: ")
    
    plot_master_data(csv_filename)