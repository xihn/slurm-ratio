# slurm-ratio

Job Submit plugin to enfornce the CPU/GPU ratio to facilitate accounting.

### Testing

For testing without compiling slurm. 
1. ```cd tests ``` and run ```gcc print.c```.
2. ```./a.out "partition" "gpu:type:count" "cpu count"``` (ex: `./a.out es1 gpu:A100:2 4`)

For testing in a docker slurm enviorment `running ./deploydocker.sh` should get you most of the way.
You can then compile the plugin within the containerized cluster, however since the docker containers
do not have GPUs you can only test so much. 


### Configuration
 The config file accepts the following settings which must all be defined under `[gresratio]`
- `EnforceRatio` will enforce a ratio using the weights of each card
- `DefaultCard` is the default card used if user does not specify a card on job submittal
- `Partition` is the partition to check 
- `card.*` is the expected ratio of different GPUs

 When the plugin is enabled, the jobs ratio is calculated by `cpu count / gpu count` which is checked against the ratio found in `card.*`.  For example if a user submits a job of `gpu:V100:4 ncpu = 4` and `card.V100 = 1` then the ratio is `4 / 4` which is equal to `1`, so the job is accepted. 

### Compiling with slurm

`gcc -shared -fPIC -pthread -I${SLURM_SRC_DIR} job_submit_require_cpu_gpu_ratio.c -o job_submit_require_cpu_gpu_ratio.so`

### TODO
- Rust rewrite?
- Handle unspecified gpu (ex. `gpu:2`) better rather than assuming `DefaultCard`. This could be a large issue.

`squeue -O "UserName,tres-per-node,MinCpus,Partition, JobID" | awk '$2 != "N/A"'`

`scontrol show job ID`

`scontrol show nodes | grep -E 'NodeName=|Gres=' | grep -B 1 'Gres=[gpu:]'`

`scontrol show node n0001.es1`
