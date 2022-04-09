#include "HighResolution.h"
#include <Logger.h>
#include <filesystem>

#define Mahamadou 0

namespace AI::ImageProcessingUnit2
{
    using namespace InferenceEngine;

    HighResNet::HighResNet(const Core& ie, const std::string& deviceName, const std::string& xmlModel, const std::string& inputImagePath, const std::string& outputImagePath) :
        ie_{ ie },
        inputImagePath_{ inputImagePath },
        outputImagePath_{ outputImagePath }
    {
        auto network = ie.ReadNetwork(xmlModel);

        // Check input
        InputsDataMap inputInfo(network.getInputsInfo());

        if (inputInfo.size() != 1)
        {
            throw std::invalid_argument("HighRes model should have only one input");
        }

        inputBlobName_ = inputInfo.begin()->first;

        const InputInfo::Ptr& inputInfoFirst = inputInfo.begin()->second;

        inputInfoFirst->getPreProcess().setResizeAlgorithm(RESIZE_BILINEAR);
        inputInfoFirst->setLayout(Layout::NHWC);
        inputInfoFirst->setPrecision(Precision::U8);

        // Check output
        OutputsDataMap outputInfo(network.getOutputsInfo());

        if (outputInfo.size() != 1)
        {
            throw std::invalid_argument("HighRes model should have only one output");
        }

        outputBlobName_ = outputInfo.begin()->first;

        const DataPtr& output = outputInfo.begin()->second;

        if (const auto outputDims = output->getTensorDesc().getDims(); outputDims.size() != 4)
        {
            throw std::length_error("Incorrect ouput dimensions for SSD");
        }
        else
        {
            size_t num_images = outputDims[0];
            size_t num_channels = outputDims[1];
            H_ = outputDims[2];
            W_ = outputDims[3];
            nPixels_ = W_ * H_;

            std::stringstream ss;
            ss << "Output size [N,C,H,W]: " << num_images << ", " << num_channels << ", " << H_ << ", " << W_;
            Common::Logging::Logger::log("information", "HighRes", -1, ss.str());
        }

        output->setPrecision(Precision::FP32);

        net_ = ie_.LoadNetwork(network, deviceName);

        /* Read input image to a blob and set it to an infer request without resize
         * and layout conversions. */
        cv::Mat image = imreadW(inputImagePath_);

        // Convert input image to YCbCr (Y -> greyscale image, Cb -> blue-difference, Cr -> chroma components)
        cv::Mat img_YCbCr;
        cv::cvtColor(image, img_YCbCr, cv::COLOR_BGR2YCrCb);

        std::vector<cv::Mat> ycc_Planes;
        cv::split(img_YCbCr, ycc_Planes);

        // Allocate memory for the individual channels (grey_, Cb_, Cr_)
        grey_.create(ycc_Planes[0].rows, ycc_Planes[0].cols, ycc_Planes[0].depth());
        Cb_.create(ycc_Planes[1].rows, ycc_Planes[1].cols, ycc_Planes[1].depth());
        Cr_.create(ycc_Planes[2].rows, ycc_Planes[2].cols, ycc_Planes[2].depth());
        ycc_Planes[0].copyTo(grey_);
        ycc_Planes[1].copyTo(Cb_);
        ycc_Planes[2].copyTo(Cr_);

#if Mahamadou
        // Show image
        cv::namedWindow("Original input image", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("Grey input image", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("Cb input image", cv::WINDOW_AUTOSIZE);
        cv::namedWindow("Cr input image", cv::WINDOW_AUTOSIZE);

        cv::imshow("Original input image", image);
        cv::imshow("Grey input image", grey_);
        cv::imshow("Cb input image", Cb_);
        cv::imshow("Cr input image", Cr_);

        cv::waitKey();
        cv::destroyAllWindows();
#endif
    }

    InferRequest HighResNet::createAndRunInferRequest()
    {
        auto inferRequest = net_.CreateInferRequest();

        Blob::Ptr imgBlob = wrapMat2Blob(grey_);                     // just wrap Mat data by Blob::Ptr
                                                                     // without allocating of new memory
        inferRequest.SetBlob(inputBlobName_, imgBlob);               // infer_request accepts input blob of any size

        // --------------------------- Step 7. Do inference ------------------------------------------------
        /* Running the request synchronously */
        inferRequest.Infer();

        return inferRequest;
    }

    cv::Mat HighResNet::imreadW(const std::string& input_image_path) const
    {
        cv::Mat image;
        std::ifstream input_image_stream;
        input_image_stream.open(input_image_path.c_str(), std::iostream::binary | std::ios_base::ate | std::ios_base::in);
        if (input_image_stream.is_open()) {
            if (input_image_stream.good()) {
                input_image_stream.seekg(0, std::ios::end);
                std::size_t file_size = input_image_stream.tellg();
                input_image_stream.seekg(0, std::ios::beg);
                std::vector<char> buffer(0);
                std::copy(std::istreambuf_iterator<char>(input_image_stream), std::istreambuf_iterator<char>(), std::back_inserter(buffer));
                image = cv::imdecode(cv::Mat(1, static_cast<int>(file_size), CV_8UC1, &buffer[0]), cv::IMREAD_COLOR);
            }
            else
            {
                std::stringstream ss;
                ss << "Input file '" << input_image_path.c_str() << "' processing error";
                Common::Logging::Logger::log("error", "HighRes", -1, ss.str());
            }
            input_image_stream.close();
        }
        else
        {
            std::stringstream ss;
            ss << "Unable to read input file '" << input_image_path.c_str() << "'";
            Common::Logging::Logger::log("error", "HighRes", -1, ss.str());
        }
        return image;
    }

    void HighResNet::getResults(InferRequest& inferRequest)const
    {
        // --------------------------- Step 8. Process output ----------------------------------------------
        MemoryBlob::CPtr output = as<MemoryBlob>(inferRequest.GetBlob(outputBlobName_));

        if (!output)
        {
            throw std::length_error("We expect output to be inherited from MemoryBlob, but by fact we were not able to cast it to MemoryBlob");
        }

        // locked memory holder should be alive all time while access to its buffer
        // happens
        auto lmoHolder = output->rmap();
        const auto output_data = lmoHolder.as<const PrecisionTrait<Precision::FP32>::value_type*>();

        // Normalize image
        std::vector<float> out;
        std::copy(output_data, output_data + nPixels_, std::back_inserter(out));
        cv::Mat out_cv(static_cast<int>(W_), static_cast<int>(H_), CV_32F, static_cast<void *>(out.data()));
        cv::Mat out_grey;
        cv::convertScaleAbs(out_cv, out_grey); // Convert to uint8

        // Convert to color image
        cv::Mat Cb_resized;
        cv::Mat Cr_resized;
        cv::Size dsize;
        dsize.width = static_cast<int>(W_);
        dsize.height = static_cast<int>(H_);

        // Resize Cb and Cr images to the final image size
        cv::resize(Cb_, Cb_resized, dsize, 0, 0, cv::INTER_CUBIC);
        cv::resize(Cr_, Cr_resized, dsize, 0, 0, cv::INTER_CUBIC);

        // Merge channels and convert to BGR image format
        std::vector<cv::Mat> out_vec(3);
        out_vec[0] = out_grey;
        out_vec[1] = Cb_resized;
        out_vec[2] = Cr_resized;

        cv::Mat YCbCrImage;
        cv::merge(out_vec, YCbCrImage);

        cv::Mat BGRImage;
        cv::cvtColor(YCbCrImage, BGRImage, cv::COLOR_YCrCb2BGR);

#if Mahamadou
        // Show image
        cv::namedWindow("Output image", cv::WINDOW_AUTOSIZE);
        cv::imshow("Output image", out_grey);

        cv::namedWindow("Color image", cv::WINDOW_AUTOSIZE);
        cv::imshow("Color image", BGRImage);

        cv::waitKey();
        cv::destroyAllWindows();
#endif
        //const auto& filename = std::filesystem::path(inputImagePath_).filename().u8string();
        //const auto& parent_path = std::filesystem::path(inputImagePath_).parent_path();
        //std::stringstream ss;
        //ss << parent_path.u8string() + "/out_horse.jpg";
        if (!cv::imwrite(/*ss.str()*/ outputImagePath_, BGRImage))
        {
            Common::Logging::Logger::log("error", "HighRes", -1, "Failed to save final image: " + /*ss.str()*/ outputImagePath_);
        }
    }


    void RunHighResNet::processImage(const void* pSender, InputArgList& args)
    {
        // ---------------------------  Start inference -------------------------------------
        InferenceEngine::Core ie;
        AI::ImageProcessingUnit2::HighResNet net(ie, args.deviceName_, args.xmlModel_, args.inputImagePath_, args.outputImagePath_);
        auto inferRequest = net.createAndRunInferRequest();
        net.getResults(inferRequest);
    }

}