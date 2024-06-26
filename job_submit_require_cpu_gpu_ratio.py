import re
from slurm_plugin import JobSubmitPlugin

# Global variables
plugin_name = "Require CPU/GPU ratio"
plugin_type = "job_submit/require_cpu_gpu_ratio"
plugin_version = "your_version_here"

# Number of partitions to be checked - need to modify
npart = 1
# The Partition to be checked - need to modify
mypart = ["es1"]
# The CPU/GPU ratio that is checked against - need to modify
ratio = [2]

# GRES GPU regex
gpu_regex = re.compile(r"^gpu:[_a-zA-Z0-9:]*([0-9]+)$")

def _str2int(str_value):
    try:
        return int(str_value)
    except ValueError:
        return None

def _check_ratio(part, gres, ncpu):
    if part is None:
        print(f"{plugin_name}: missed partition info")
        return True

    # Loop through all partitions that need to be checked
    for i in range(npart):
        if part == mypart[i]:
            # Require GRES on a GRES partition
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
                    if ncpu / ngpu < ratio[i]:
                        print(f"{plugin_name}: CPU={ncpu}, GPU={ngpu}, not qualify")
                        return False
                else:
                    print(f"{plugin_name}: missed GPU on partition {mypart[i]}")
                    return False

    return True

class MyJobSubmitPlugin(JobSubmitPlugin):
    def job_submit(self, job_desc, submit_uid):
        if not _check_ratio(job_desc['partition'], job_desc.get('gres'), job_desc['min_cpus']):
            return {'err_code': -1, 'err_msg': 'Invalid CPU/GPU ratio'}
        return {'err_code': 0}

    def job_modify(self, job_desc, job_ptr, submit_uid):
        partition = job_desc.get('partition', job_ptr['partition'])
        gres = job_desc.get('gres', job_ptr['gres'])
        min_cpus = job_desc.get('min_cpus', job_ptr['total_cpus'])
        if not _check_ratio(partition, gres, min_cpus):
            return {'err_code': -1, 'err_msg': 'Invalid CPU/GPU ratio'}
        return {'err_code': 0}
