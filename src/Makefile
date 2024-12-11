SLURM_INC = /usr/include/slurm/
SLURM_LIB = /usr/lib64/

CC = gcc
CFLAGS = -fPIC -shared -I$(SLURM_INC) -Wall
LDFLAGS = -L$(SLURM_LIB) -lslurm

PLUGIN = job_submit_require_cpu_gpu_ratio.so

all: $(PLUGIN)

$(PLUGIN): job_submit_require_cpu_gpu_ratio.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(PLUGIN)
