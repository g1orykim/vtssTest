BUILD INSTRUCTIONS
==================
This needs to be build as a part of the API, but must be explicit
enabled:

cd ../../
mkdir build
cd build
cmake -DVTSS_APPL_HOSTSIM=on -DVTSS_PHY_API_ONLY=ON ..
make -j4


