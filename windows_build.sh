mkdir build
C:/msys64/mingw64/bin/g++ -fdiagnostics-color=always -g main.cpp tripview_api.cpp -I C:/include/curlpp-0.8.1/include -L C:/include/curlpp-0.8.1/build -lcurlpp -o build/tripsit.exe -lcurl `wx-config --cxxflags --libs`
cp -a C:/include/curlpp-0.8.1/build/libcurlpp.dll build
cp -a stations.txt build
touch build/trips.txt