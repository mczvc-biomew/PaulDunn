package io.github.yumika;

import com.badlogic.gdx.ApplicationAdapter;
import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.Input;
import com.badlogic.gdx.graphics.*;
import com.badlogic.gdx.graphics.g2d.SpriteBatch;
import com.badlogic.gdx.graphics.glutils.ShaderProgram;

import com.badlogic.gdx.utils.BufferUtils;
import com.badlogic.gdx.utils.ScreenUtils;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;

/** {@link com.badlogic.gdx.ApplicationListener} implementation shared by all platforms. */
public class Main extends ApplicationAdapter {
    private SpriteBatch batch;
    private int glowTextureID;
    ShaderProgram shaderProgram;
    FloatBuffer positions;
    FloatBuffer colors;

    int NUM_PARTICLES = 20666;
    float width = 0.7f;
    float height = 3.5f;

    float x = 0.0f;
    float y = 0.0f;
    int ww;
    int wh;
    int boxSize;
    final float aspect = 1440.0f / 900.0f;

    float minX = 0.5f;
    float minY = minX * aspect;

    float maxX = 5.5f;
    float maxY = (maxX + 0.5f) * aspect;

    float caw = maxX - minX;
    float cah = maxY - minY;
    float daw = width * 2 / caw;
    float dah = height / cah;

    float t = 3.0f;

    double PHI = (1 + Math.sqrt(5)) / 2;
    double TAU = 6.2831855;

    void loadTextures() {
        glowTextureID = Gdx.gl.glGenTexture();

        Gdx.gl.glBindTexture(GL20.GL_TEXTURE_2D, glowTextureID);
        PCMLoader.Image glowImage = null;
        try {
            glowImage = PCMLoader.loadTexture("glow_image.pcm");
        } catch (IOException e) {
            System.err.println("Error loading glow image.");
            throw new RuntimeException(e);
        }

        ByteBuffer textureBuffer = glowImage.getData();
        Gdx.gl.glTexImage2D(GL20.GL_TEXTURE_2D,
            0, GL20.GL_RGBA,
            glowImage.getWidth(), glowImage.getHeight(),
            0, GL20.GL_RGBA,
            GL20.GL_UNSIGNED_BYTE, textureBuffer);
        Gdx.gl.glTexParameterf(GL20.GL_TEXTURE_2D, GL20.GL_TEXTURE_WRAP_S, GL20.GL_REPEAT);
        Gdx.gl.glTexParameterf(GL20.GL_TEXTURE_2D, GL20.GL_TEXTURE_WRAP_T, GL20.GL_REPEAT);
        Gdx.gl.glTexParameterf(GL20.GL_TEXTURE_2D, GL20.GL_TEXTURE_MIN_FILTER, GL20.GL_LINEAR);
        Gdx.gl.glTexParameterf(GL20.GL_TEXTURE_2D, GL20.GL_TEXTURE_MAG_FILTER, GL20.GL_LINEAR);
    }

    void loadShaders() {
        positions = BufferUtils.newFloatBuffer(NUM_PARTICLES * 2);
        colors = BufferUtils.newFloatBuffer(NUM_PARTICLES * 3);
        positions.rewind();
        colors.rewind();

        shaderProgram = new ShaderProgram(
            Gdx.files.internal("vertex.glsl").readString(),
            Gdx.files.internal("fragment.glsl").readString()
        );

        shaderProgram.bind();
        shaderProgram.enableVertexAttribute("position");
        shaderProgram.setVertexAttribute("position", 2, GL20.GL_FLOAT, false, 0, positions);
        shaderProgram.enableVertexAttribute("color");
        shaderProgram.setVertexAttribute("color", 3, GL20.GL_FLOAT, false, 0, colors);

        shaderProgram.setUniformi("u_texture", 0);
        shaderProgram.setUniformf("sensitivity", 99.0f/255.0f);

    }

    void enableGLRendering() {
        Gdx.gl20.glEnable(GL20.GL_BLEND);
        Gdx.gl20.glBlendFunc(GL20.GL_SRC_ALPHA, GL20.GL_ONE);
        Gdx.gl20.glActiveTexture(GL20.GL_TEXTURE0);
        Gdx.gl20.glBindTexture(GL20.GL_TEXTURE_2D, glowTextureID);
        Gdx.gl20.glEnable(GL20.GL_TEXTURE_2D);

        Gdx.graphics.setVSync(true);
        Gdx.gl20.glClearDepthf(1.0f);
        Gdx.gl20.glDisable(GL20.GL_DEPTH_TEST);
        Gdx.gl20.glDepthFunc(GL20.GL_LEQUAL);
        Gdx.gl20.glDisable(GL20.GL_CULL_FACE);
        Gdx.gl20.glDisable(GL20.GL_STENCIL_TEST);
        Gdx.gl20.glDisable(GL20.GL_DITHER);
        Gdx.gl.glEnable(GL20.GL_VERTEX_PROGRAM_POINT_SIZE);
    }

    @Override
    public void create() {
        batch = new SpriteBatch();

        ww = Gdx.graphics.getWidth();
        wh = Gdx.graphics.getHeight();
        boxSize = Math.min(ww, wh);

        loadTextures();
        loadShaders();

        enableGLRendering();
    }

    void step() {

        // VERDICT: glDrawArrays is slow in this environment; 06-18-2025
        // RESOLVED! 06-19-2025 (2158H)
        double j = 0.0;
        for (int i = 0; i < NUM_PARTICLES; i++) {
            // PaulDunn, creator of SpecBasic, interpreter for SinClair Basic -- Bubble Universe.
            final float u = (float)(Math.sin(i + y) + Math.sin((j / (NUM_PARTICLES * Math.PI)) + x));
            final float v = (float)(Math.cos(i + y) + Math.cos(j / (NUM_PARTICLES * Math.PI) + x));
            x = u + t;
            y = v + t;

            Color color = Color.createHue(
                Math.cos(
                    Math.cos(i)
                        - Math.sin(t*PHI*PHI*PHI) )
            );

            double vX = (u - minX + 0.5) * daw ;
            double vY = (v - minY + 0.5) * dah + 0.125;

            int vI = i * 2;
            positions.put((float) vX);
            positions.put((float) vY);

            int cI = i * 3;
            colors.put((float) color.r);
            colors.put((float) color.g);
            colors.put((float) color.b);

            j += t;
        }
        t += 1.0f / 240.0f;
    }

    @Override
    public void render() {
        Gdx.gl20.glViewport(0, 0, Gdx.graphics.getWidth(), Gdx.graphics.getHeight());
        ScreenUtils.clear(0.15f, 0.15f, 0.2f, 1f);

        step();
        positions.flip();
        colors.flip();


        Gdx.gl20.glDrawArrays(GL20.GL_POINTS, 0, NUM_PARTICLES);


        if (Gdx.input.isKeyJustPressed(Input.Keys.ESCAPE)) {
            Gdx.app.exit();
        };
    }

    @Override
    public void dispose() {
        batch.dispose();
    }

    static class Color {
        double r;
        double g;
        double b;

        Color(double red, double green, double blue) {
            this.r = red;
            this.g = green;
            this.b = blue;
        }

        Color add(Color rhs) {
            this.r += rhs.r;
            this.g += rhs.g;
            this.b += rhs.b;
            return this;
        }

        static Color createHue(double h) {
            h *= 6.0;

            int hi = (int)h;
            double hf = h - hi;

            switch ((int) (double) (hi % 6)) {
                case 0:
                    return new Color(1.0, hf, 0.0);
                case 1:
                    return new Color(1.0 - hf, 1.0, 0.0);
                case 2:
                    return new Color(0.0, 1.0, hf);
                case 3:
                    return new Color(0.0, 1.0 - hf, 1.0);
                case 4:
                    return new Color(hf, 0.0, 1.0);
                case 5:
                    return new Color(1.0, 0.0, 1.0 - hf);
            }

            return new Color(0, 0, 0);
        }

    };
}
