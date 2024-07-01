# slurm-ratio
Slurm Job Submit Plugin for enforcing a CPU/GPU ratio



loop through all 

partition


consts

1080ti
2080ti
v100
a40
p100
k80


flowchart 
job_desc->partition,
job_desc->tres_per_node,
job_desc->min_cpus


pull from regex -> gpu type -> 
var gpu type = 2080ti
lookup in JSON file for name=vargputype
if ratio min or max fits then slurm accept
if not slurm deny


