# httpServer
Build command for the "release" version
1- cmake -G 'Visual Studio 15 2017 Win64' -DCMAKE_TOOLCHAIN_FILE=C:/temp/vcpkg/scripts/buildsystems/vcpkg.cmake 
	-DCMAKE_PREFIX_PATH="D:\Pytorch\libtorch-win-shared-with-deps-1.10.2+cpu\libtorch;C:\Users\bijou\Desktop\PocoServer\PocoServer\src\AI_Image_Processing_Unit2\deployment_tools\inference_engine;C:\Users\bijou\Desktop\PocoServer\PocoServer\src\AI_Image_Processing_Unit2\deployment_tools\opencv" 
	-DCMAKE_BUILD_TYPE=Release ..

2- cmake --build . --config=Release --verbose 



Build command for the "Debug" version (Not working correctly)
1- cmake -G 'Visual Studio 15 2017 Win64' -DCMAKE_TOOLCHAIN_FILE=C:/temp/vcpkg/scripts/buildsystems/vcpkg.cmake 
	-DCMAKE_PREFIX_PATH="D:\Pytorch\libtorch-win-shared-with-deps-debug-1.10.2+cpu\libtorch;C:\Users\bijou\Desktop\PocoServer\PocoServer\src\AI_Image_Processing_Unit2\deployment_tools\inference_engine;C:\Users\bijou\Desktop\PocoServer\PocoServer\src\AI_Image_Processing_Unit2\deployment_tools\opencv" 
	-DCMAKE_BUILD_TYPE=Debug ..

2- cmake --build . --config=Debug --verbose 


Convert pytorch model to ONNX representation OpenVINO IR (Intermediate Representation)
python "C:\Program Files (x86)\Intel\openvino_2021\deployment_tools\open_model_zoo\tools\downloader\converter.py" 
--mo "C:\Program Files (x86)\Intel\openvino_2021\deployment_tools\model_optimizer\mo.py" --name "squeezenet1.1" 
-d "C:\Users\bijou\Documents\Intel\OpenVINO\openvino_models\models" 
-o "C:\Users\bijou\Documents\Intel\OpenVINO\openvino_models\ir" --precisions "FP16"
