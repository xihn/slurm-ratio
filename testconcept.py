import re


def request(gres, cpu):
    # Preassigned weights based on relative computational power
    gpu_weights = {
        '2080ti': 2, # 2 per gpu
        'v100': 4,   # 4 per gpu  -- should maybe be 8 since there are only 2 nodes with 16 cores and 2x v100
        'a40': 16 # 16 per gpu
        # Add more GPUs and their weights as needed
    }
    
    # Extract GPU type and count from 'gres'

    parts = gres.split(':')
    gpu_type = parts[1] if len(parts) > 1 and not parts[1].isnumeric() else None
    gpu_count = int(parts[-1])
    
    # Calculate combined GPU power based on weights
    if gpu_type:
        if gpu_type.lower() in gpu_weights:
            ratio = gpu_weights[gpu_type.lower()] * gpu_count
        else:
            raise ValueError(f"Unknown GPU type: {gpu_type}")
    else:
        # Assuming a default GPU type (e.g., v100) if not specified
        ratio = gpu_weights['v100'] * gpu_count
    
    # Calculate the maximum allowed CPU cores based on GPU power
    max_cpu_cores = ratio * 2  # Adjust as per your requirement
    
    return cpu <= ratio


print(request('gpu:2', 8)) 
