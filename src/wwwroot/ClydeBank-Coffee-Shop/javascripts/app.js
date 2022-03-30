$(document).ready(function () {

    inputImg.onchange = evt => {
        const [file] = inputImg.files
        if (file) {
          preview.src = URL.createObjectURL(file)
        }
      }

    inputImg2.onchange = evt => {
        const [file] = inputImg2.files
        if (file) {
            preview.src = URL.createObjectURL(file)
        }
    }

    $("#horseZebra").submit(function (e) {

        //disable the submit button
        $("#btnHorseZebra").attr("disabled", true);

        return true;

    });

    $("#highRes").submit(function (e) {

        //disable the submit button
        $("#btnHighRes").attr("disabled", true);

        return true;

    });
});