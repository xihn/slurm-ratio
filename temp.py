import re
import json

gpu_regex = re.compile(r"^gpu:[_a-zA-Z0-9:]*([0-9]+)$")
npart = 1

def check_ratio1(part, gres, ncpu):
    if part is None:
        #print error no partition given
        return True

    # Loop through all partitions that need to be checked
    for i in range(npart):
        if gres is None:
            print(f"{plugin_name}: missed GRES on partition {mypart[i]}")
            return False
        else:
            match = gpu_regex.match(gres)
            if match:
                ngpu = _str2int(match.group(1))
                if ngpu is None or ngpu < 1:
                    print(f"{plugin_name}: invalid GPU number {match.group(1)}")
                    return False

                # Sanity check of the CPU/GPU ratio
                given = ncpu / ngpu 

                if ncpu / ngpu < ratio[i]:
                    print(f"{plugin_name}: CPU={ncpu}, GPU={ngpu}, not qualify")
                    return False
            else:
                print(f"{plugin_name}: missed GPU on partition {mypart[i]}")
                return False

    return True

# Assuming your JSON data is stored in a file named 'gpu_data.json'
with open('gpu_cpu_ratios.json', 'r') as f:
    gpu_data = json.load(f)

def gpu_weight(gpu):
    for gpu_entry in gpu_data:
        if gpu_entry.get('name') == gpu:
            return int(gpu_entry.get('weight', 1))  # Return weight as integer, default to 1 if not present
    ValueError

def gpu_max(gpu):
    for gpu_entry in gpu_data:
        if gpu_entry.get('name') == gpu:
            return int(gpu_entry.get('max', 1))  # Return max value as integer, default to 1 if not present
    ValueError

def get_ratio(gpu, cpu):
    return cpu / (gpu.num * gpu_weight(gpu))

def check_ratio(partition, gres, numcpu):
    if partition is None:
        #print error no part given
        return True
    
    for i in range(npart):
        #do some shit
        1
    
    return None
