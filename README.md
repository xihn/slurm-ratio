# slurm-ratio
Slurm Job Submit Plugin for enforcing a CPU/GPU ratio



loop through all 

partition


int maxA40 = 16;
int maxV100 = 4;
int max2080TI = 2;


8cpu/1gpu v = 8 which is > 2
4cpu / 2gpu = 2 which is not > 2 so fail



example GRES: gpu:GRTX2080TI:4
or gpu:2

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


./program "es1" "gpu:GRTX2080TI:4" "1"

actual design:

from settings read first 4 lines and set env variables  
    EnforceRatio = 
    DefaultCard = 
    EnforceMin =
    EnforceMax =
    Partition = 


from user input get partitions, gres, ncpu
basic checks for validity
    - loop through partitions
    - etc etc, then once find match for GPU name and number
    - if gpu name not present just use DefaultCard
    - update global vars for max min and ratio


    - if EnforceMin check min
    - if EnforceMax check max
    - if EnforceRatio check ratio
    - else all fails just slurm accept

example check in pseudocode
if (enforcement not met eg ncpu < min) {
    return ERROR
}

 