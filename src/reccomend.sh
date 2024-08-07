# Simple script that pulls GPU and CPU information of SLURM nodes
# buld hashmap 

# node -> ncpu, ngpu, name

# mismatched nodes -> or outlier nodes

# nameofcard -> touple [ngpu, ncpu]
# 
# for i in cards:
#     print(i.ngpu / i.ncpu)

# -A Flag here is a hashmap 
declare -A nodes


# -A means associative array (hashmap)
# -r means read-only
declare -A -r FLY_REGIONS=(
  ["a100-40gb"]="ord"
  ["a100-80gb"]="mia"
  ["l40s"]="ord"
)


unset FLY_REGIONS["a100-40gb"]

unset FLY_REGIONS