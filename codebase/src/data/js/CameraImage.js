const ColourTypes = new Set("J", "2GS", "4GS", "8GS", "8C", "12C", "16C");
const ResolutionWidths = new Set([80, 128, 160, 320, 640]);
const ResolutionHeights = new Set([60, 64, 96, 120, 128, 240, 480]);

class CameraImage {
    #colour_type;
    #width;
    #height;
    #data;
    #image_uri;
    #image_blob;

    constructor() {
        this.#colour_type = "";
        this.#width = 0;
        this.#height = 0;
        this.#data = "";
        this.#image_uri = "";
        this.#image_blob = null;
    }

    clearData() {
        this.#data = "";
    }

    setData(data) {
        this.#data = data;
    }

    appendData(data) {
        this.#data += data;
    }

    setColourType(type) {
        if (ColourTypes.has(type)) {
            this.#colour_type = type;
            return true;
        }
        return true;
    }

    setResolution(width, height) {
        if (ResolutionWidths.has(width) && ResolutionHeights.has(height)) {
            this.#width = width;
            this.#height = height;
            return true;
        }
        return false;
    }

    getImageUri() {
        return this.#image_uri;
    }

    validImageData() {
        let invalid_chars = this.#data.match(/[^A-Fa-f0-9]/g);
        if (invalid_chars) {
            view.addConsoleText("Invalid characters in image data: " + invalid_chars);
            return false;
        }
        return true;
    }

    processImageData() {
        const bit_depth = parseInt(this.#colour_type, 10);
        let regex = /[a-f0-9]{1,2}/gi;  // for JPEG or 8-bit images
        if ((bit_depth == 2) || (bit_depth == 4))
            regex = /[a-f0-9]/gi;
        else if (bit_depth == 12)
            regex = /[a-f0-9]{1,3}/gi;
        else if (bit_depth == 16)
            regex = /[a-f0-9]{1,4}/gi;

        const img_vals = Uint8Array.from(this.#data.match(regex), x => parseInt(x, 16));

        if (this.#colour_type == "J") {
            this.#image_blob = new Blob([img_vals], { type: 'application/octet-stream' });
            this.#image_uri = window.URL.createObjectURL(this.#image_blob);
        }
        else {
            let buffer = new Uint8ClampedArray(this.#width * this.#height * 4);

            let count = 0;
            if (this.#colour_type == "2GS" || this.#colour_type == "4GS" || this.#colour_type == "8GS") {
                for (let y = 0; y < this.#height; y++) {
                    for (let x = 0; x < this.#width; x++) {
                        let val = img_vals[count];
                        if (bit_depth == 2)
                            val = val << 6;
                        else if (bit_depth == 4)
                            val = val << 4;

                            let pos = (y * width + x) * 4;
                        buffer[pos] = val;
                        buffer[pos+1] = val;
                        buffer[pos+2] = val;
                        buffer[pos+3] = 255;
                        if ( (this.#colour_type != "2GS") || (count % 2 == 0) )
                            count += 1;
                    }
                }
            }
            else {
                // for 8-bit colour images
                let red_mask = 0b11100000;
                let red_shift = 0;
                let green_mask = 0b00011100;
                let green_shift = 3;
                let blue_mask = 0b00000011;
                let blue_shift = 6;

                if (colour_type == "12C") {
                    red_mask = 0b111100000000;
                    red_shift = 4;
                    green_mask = 0b000011110000;
                    green_shift = 0;
                    blue_mask = 0b000000001111;
                    blue_shift = 4;
                }
                else if (colour_type == "16C") {
                    red_mask = 0b1111100000000000;
                    red_shift = 8;
                    green_mask = 0b0000011111100000;
                    green_shift = 3;
                    blue_mask = 0b0000000000011111;
                    blue_shift = 3;
                }

                for (let y = 0; y < this.#height; y++) {
                    for (let x = 0; x < this.#width; x++) {
                        let pos = (y * this.#width + x) * 4;

                        buffer[pos] = (img_vals[count] & red_mask) >> red_shift;
                        const g = img_vals[count] & green_mask;
                        buffer[pos+1] = (colour_type == "16C") ? (g >> green_shift) : (g << green_shift);
                        buffer[pos+2] = (img_vals[count] & blue_mask) << blue_shift;
                        buffer[pos+3] = 255;

                        count += 1;
                    }
                }
            }

            let canvas = document.createElement('canvas');
            canvas.width = this.#width;
            canvas.height = this.#height;

            let canvas_context = canvas.getContext('2d');
            let image_data = canvas_context.createImageData(this.#width, this.#height);
            image_data.data.set(buffer);
            canvas_context.putImageData(image_data, 0, 0);

            this.#image_uri = canvas.toDataURL();
        }

        return true;
    }
}