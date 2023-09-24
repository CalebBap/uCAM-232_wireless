const ImageTypes = Object.freeze({
    NONE: Symbol('none'),
    JPG: Symbol('jpg'),
    GS2: Symbol('GS2'),
    GS4: Symbol('GS4'),
    GS8: Symbol('GS8'),
    C08: Symbol('C08'),
    C12: Symbol('C12'),
    C16: Symbol('C16')
});

const TypeIdentifiers = new Map([
    [new Uint8Array([0x4A, 0x46, 0x49, 0x46, 0x00]), ImageTypes.JPG],
    [new Uint8Array([0x47, 0x53, 0x32, 0x00, 0x00]), ImageTypes.GS2],
    [new Uint8Array([0x47, 0x53, 0x34, 0x00, 0x00]), ImageTypes.GS4],
    [new Uint8Array([0x47, 0x53, 0x38, 0x00, 0x00]), ImageTypes.GS8],
    [new Uint8Array([0x43, 0x30, 0x38, 0x00, 0x00]), ImageTypes.C08],
    [new Uint8Array([0x43, 0x31, 0x32, 0x00, 0x00]), ImageTypes.C12],
    [new Uint8Array([0x43, 0x31, 0x36, 0x00, 0x00]), ImageTypes.C16],
]);

const HeaderOffsets = {
    SOI: 0,
    RAW_DIMENSIONS: 2,
    IDENTIFER: 6,
    END: 11
};

class CameraImage {
    #raw_data;
    #image_type;
    #uri;

    constructor() {
        this.resetData();
        this.#uri = '';
    }

    resetData() {
        this.#raw_data = new Uint8Array();
        this.#image_type = ImageTypes.NONE;
    }

    processData(data) {
        if (this.isStartOfImage(data)) {
            this.#raw_data = data;
        }
        else {
            this.appendImageData(data);
        }

        if (this.#raw_data.length === 0) {
            return;
        }

        if (!this.getType()) {
            return;
        }

        if (this.endOfImage(data)) {
            this.makeUri();
            return true;
        }

        return false;
    }

    isStartOfImage(data) {
        const soi_bytes = new Uint8Array([0xFF, 0xD8]);
        if (data.length < soi_bytes.length) {
            return false;
        }
        return soi_bytes.every((val, i) => val === data[i]);
    }

    appendImageData(data) {
        let new_data = new Uint8Array(this.#raw_data.length + data.length);
        new_data.set(this.#raw_data);
        new_data.set(data, this.#raw_data.length);
        this.#raw_data = new_data;
    }

    getType() {
        if (this.#image_type !== ImageTypes.NONE) {
            return true;
        }

        if (this.#raw_data.length < HeaderOffsets.END) {
            return false;
        }

        for (const [key, value] of TypeIdentifiers) {
            if (key.every((byte, i) => byte === this.#raw_data[i + HeaderOffsets.IDENTIFER])) {
                this.#image_type = value;
                return true;
            }
        }

        return false;
    }

    endOfImage(data) {
        const eoi_bytes = new Uint8Array([0xFF, 0xD9]);

        if (this.#image_type !== ImageTypes.JPG) {
            return (data.length == eoi_bytes.length) && data.every((byte, i) => byte === eoi_bytes[i]);
        }

        for (let i = (this.#raw_data.length - 1); i > 0; i--) {
            if (this.#raw_data[i] !== 0) {
                const end_bytes = [this.#raw_data[i - 1], this.#raw_data[i]];
                return end_bytes.every((byte, i) => byte === eoi_bytes[i]);
            }
        }

        return false;
    }

    makeUri() {
        if (this.#image_type === ImageTypes.JPG) {
            const blob = new Blob([this.#raw_data], { type: 'application/octet-stream' });
            this.#uri = window.URL.createObjectURL(blob);
        }
        else {
            this.processRawData();
        }
        this.resetData();
    }

    getUri() {
        return this.#uri;
    }

    processRawData() {
        let count = HeaderOffsets.RAW_DIMENSIONS;
        const width = (this.#raw_data[count] << 8) | this.#raw_data[++count];
        const height = (this.#raw_data[++count] << 8) | this.#raw_data[++count];
        count += HeaderOffsets.END;

        let buffer = new Uint8ClampedArray(width * height * 4);
        if (this.#image_type == ImageTypes.GS2 || this.#image_type == ImageTypes.GS4 || this.#image_type == ImageTypes.GS8) {
            for (let y = 0; y < height; y++) {
                for (let x = 0; x < width; x++) {
                    let val = this.#raw_data[count];
                    if (this.#image_type === ImageTypes.GS2)
                        val = val << 6;
                    else if (this.#image_type === ImageTypes.GS4)
                        val = val << 4;

                    let pos = (y * width + x) * 4;
                    buffer[pos] = val;
                    buffer[pos+1] = val;
                    buffer[pos+2] = val;
                    buffer[pos+3] = 255;
                    if ( (this.#image_type != ImageTypes.GS2) || (count % 2 == 0) )
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

            for (let y = 0; y < height; y++) {
                for (let x = 0; x < width; x++) {
                    let pos = (y * width + x) * 4;

                    buffer[pos] = (this.#raw_data[count] & red_mask) >> red_shift;
                    const g = this.#raw_data[count] & green_mask;
                    buffer[pos+1] = (colour_type == "16C") ? (g >> green_shift) : (g << green_shift);
                    buffer[pos+2] = (this.#raw_data[count] & blue_mask) << blue_shift;
                    buffer[pos+3] = 255;

                    count += 1;
                }
            }
        }

        let canvas = document.createElement('canvas');
        canvas.width = width;
        canvas.height = height;

        let canvas_context = canvas.getContext('2d');
        let image_data = canvas_context.createImageData(width, height);
        image_data.data.set(buffer);
        canvas_context.putImageData(image_data, 0, 0);

        this.#uri = canvas.toDataURL();
    }
}