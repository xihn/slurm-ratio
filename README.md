# slurm-ratio

Job Submit plugin to enfornce the CPU/GPU ratio to facilitate accounting.
Requires a configuration YAML file which contains several settings with the name and path defined in `configFile` (line 51). The config file accepts the following settings which must all be defined within the first 5 lines.
- `EnforceRatio` will enforce a ratio using the weights of each card
- `DefaultCard` is the default card used if user does not specify a card on job submittal
- `EnforceMin` will enforce a minimum number of cpu per each card
- `EnforceMax` will enforce a maximum number of cpu per each card
- `Partition` is the partition to check 

Card information should be placed in the YAML file after the settings and in the following format 
<p align=center> <code>{"name":"GRTX2080TI", "min":2, "max":2, "weight":"2"}</code></p>

Where `name` is the cards name in slurm gres, `min`/`max` is the minimum and maximum cpu that is required per card.
`weight` is used to calculate the ratio and could be assigned by teraflops or VRAM or some sort of other preformance metric. When the ratio is enforced, the jobs ratio is calculated by multiplying the card count by its weight then dividing by total cpu count. For example if a user submits a job of `gpu:V100:4 ncpu = 4` and `weight` for the V100 card is set to `2` then the weight of the job will be `4 * 2 = 8` this is then divided by the number of cpus to determine the ratio `4/8 = 2`. A job submitted without specifying which card to be used (ex. gres `gpu:2`) will assume `DefaultCard`.


Some pitfalls 
- Errors if gres has gpu count greater than 9. I assume this won't be a problem since no node has double digit number of gpus? I also don't know how submissions work for jobs that require GPU spanning across nodes.
- The YAML parser **requires** that the config has the 5 setting variables present in the exact order of the example config. And that GPU information is on the following line
- Im fairly sure users can get around the entire plugin by submitting via the newer `--gpu*` flags rather than `--gres`. Ex user submits job as `srun --gpu 2080:2`
- The user not specifying what gpu in gres means there is some guesswork in assuming which card will be used as the default. I don't know enough about slurm and how it chooses this.
- I have no clue portability wise, think C11 or C98 is used by slurm? The original author used `uint32_t` however I just used int and double freely without memory alloc specs. I assume this won't be an issue. A bit worried about memory stuffs since i was taught rust instead of C in class. `DefaultCard[20]` and `Partition[20]` are a bit generous but its fine.

