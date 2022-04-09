#pragma once

#include <inference_engine.hpp>
#include <samples/ocv_common.hpp>

namespace AI::ImageProcessingUnit2
{
    class HighResNet
    {
    public:
        HighResNet() = default;
        HighResNet(const InferenceEngine::Core& ie, const std::string& deviceName, const std::string& xmlModel, 
            const std::string& inputImagePath, const std::string& outputImagePath);
        InferenceEngine::InferRequest createAndRunInferRequest();
        void getResults(InferenceEngine::InferRequest& inferRequest)const;

    private:
        /**
         * @brief Realization cv::imread with support Unicode paths
        */
        cv::Mat imreadW(const std::string& input_image_path)const;

        InferenceEngine::Core ie_;
        InferenceEngine::ExecutableNetwork net_;
        std::string inputImagePath_;
        std::string outputImagePath_;
        std::string inputBlobName_{};
        std::string outputBlobName_{};
        size_t H_{};
        size_t W_{};
        size_t nPixels_{};
        // Y->greyscale image, Cb->blue - difference, Cr->chroma components)
        cv::Mat grey_;
        cv::Mat Cb_;
        cv::Mat Cr_;
    };

    struct InputArgList
    {
        std::string deviceName_;
        std::string inputImagePath_;
        std::string outputImagePath_;
        std::string xmlModel_;
    };

    class RunHighResNet
    {
    public:
        void processImage(const void* pSender, InputArgList& args);
    };
}