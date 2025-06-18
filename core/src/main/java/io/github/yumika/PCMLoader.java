package io.github.yumika;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.utils.BufferUtils;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

class PCMLoader {

    public static class PCMStructLayout {
        public static final int ID_SIZE_OFFSET = 0;
        public static final int IMAGE_TYPE_OFFSET = 1;
        public static final int PALLETE_SIZE_OFFSET = 2;
        public static final int PALLETE_ENTRY_DEPTH_OFFSET = 4;
        public static final int X_OFFSET = 6;
        public static final int WIDTH_OFFSET = 8;
        public static final int Y_OFFSET = 10;
        public static final int HEIGHT_OFFSET = 12;
        public static final int MAP_TYPE_OFFSET = 14;
        public static final int COLOR_DEPTH_OFFSET = 16;
        public static final int DESCRIPTOR_OFFSET = 17;
        public static final int ALPHA_OFFSET = 18;
        public static final int R_OFFSET = 20;
        public static final int G_OFFSET = 24;
        public static final int B_OFFSET = 28;
    }

    public static class Image {
        int width;
        int height;
        ByteBuffer data;

        Image(int width, int height, ByteBuffer data) {
            this.width = width;
            this.height = height;
            this.data = data;
        }

        public int getWidth() { return width; }
        public int getHeight() { return height; }
        public ByteBuffer getData() { return data; }
    }

    public static Image loadTexture(String filePath) throws IOException {
        byte[] fileBytes = Gdx.files.internal(filePath).readBytes();
        ByteBuffer buffer = ByteBuffer.wrap(fileBytes);
        buffer.order(ByteOrder.LITTLE_ENDIAN); // Important!

        int idSize = buffer.get(PCMStructLayout.ID_SIZE_OFFSET) & 0xFF;
        int imageType = buffer.get(PCMStructLayout.IMAGE_TYPE_OFFSET) & 0xFF;
        int palleteSize = buffer.getShort(PCMStructLayout.PALLETE_SIZE_OFFSET) & 0xFFFF;
        int palleteEntryDepth = buffer.getShort(PCMStructLayout.PALLETE_ENTRY_DEPTH_OFFSET) & 0xFFFF;
        int x = buffer.getShort(PCMStructLayout.X_OFFSET) & 0xFFFF;
        int width = buffer.getShort(PCMStructLayout.WIDTH_OFFSET) & 0xFFFF;
        int y = buffer.getShort(PCMStructLayout.Y_OFFSET) & 0xFFFF;
        int height = buffer.getShort(PCMStructLayout.HEIGHT_OFFSET) & 0xFFFF;
        int mapType = buffer.getShort(PCMStructLayout.MAP_TYPE_OFFSET) & 0xFFFF;
        int colorDepth = buffer.get(PCMStructLayout.COLOR_DEPTH_OFFSET) & 0xFF;
        int descriptor = buffer.get(PCMStructLayout.DESCRIPTOR_OFFSET) & 0xFF;
        int alpha = buffer.getShort(PCMStructLayout.ALPHA_OFFSET) & 0xFFFF;
        long r = buffer.getInt(PCMStructLayout.R_OFFSET) & 0xFFFFFFFFL;
        long g = buffer.getInt(PCMStructLayout.G_OFFSET) & 0xFFFFFFFFL;
        long b = buffer.getInt(PCMStructLayout.B_OFFSET) & 0xFFFFFFFFL;

        int size = width * height * colorDepth / 16;
        int offset = PCMStructLayout.B_OFFSET + 4;

        ByteBuffer rgbaData = BufferUtils.newByteBuffer(size); // Assuming 4 bytes per pixel (RGBA)
        rgbaData.rewind();
        for (int i = 0; i < size - offset - 4; i += 4) {
            int rVal = buffer.get(offset + i) & 0xFF;
            int gVal = buffer.get(offset + i + 1) & 0xFF;
            int bVal = buffer.get(offset + i + 2) & 0xFF;
            int aVal = buffer.get(offset + i + 3) & 0xFF;

            rgbaData.put((byte) vmap(rVal, 0, 5000, -255, 2200));
            rgbaData.put((byte) vmap(gVal, 0, 5000, -255, 2200));
            rgbaData.put((byte) vmap(bVal, 0, 5000, -255, 2200));
            rgbaData.put((byte) aVal);
        }
        rgbaData.flip();

        // You can return rgbaData to feed to an OpenGL texture via LWJGL/JOGL/etc.
        return new Image(width, height, rgbaData);
    }

    private static int vmap(int value, int inMin, int inMax, int outMin, int outMax) {
        return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
    }
}
