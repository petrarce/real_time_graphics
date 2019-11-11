uniform vec3 uColor;
uniform bool uSphere;
uniform bool uPaddle;

in vec2 vRelPos;
out vec3 fColor;

void main()
{
    // The sphere is actually rendered as a quad so we discard all fragments that lie outside the sphere area
    if (uSphere && distance(vRelPos, vec2(0.5, 0.5)) > 0.5)
        discard;

    // pass color
    fColor = uColor;

    if(uSphere)
    {
        // 3D effect
        fColor *= 1 - 0.5*distance(vRelPos, vec2(0.5, 0.5));
        // fake specular highlight
        fColor = mix(fColor, 0.5 * fColor + 0.5 * vec3(1.0), smoothstep(0.3, 0.0, distance(vRelPos, vec2(0.4, 0.7))));
    }
    else if(uPaddle)
    {
        float borderX = 0.3;
        float borderY = 0.05;
        float intensity = 0.5;

        float left = mix(1, smoothstep(0, borderX, vRelPos.x), intensity);
        float right = mix(1, smoothstep(1.0, 1.0-borderX, vRelPos.x), intensity);
        float top = mix(1, smoothstep(0, borderY, vRelPos.y), intensity);
        float bottom = mix(1, smoothstep(1.0, 1.0-borderY, vRelPos.y), intensity);

        // 3D effect
        fColor *= min(min(left, right), min(top, bottom));
    }
    else
    {
        // create soccer-like lines on the playground
        float lineThickness = 0.003;
        float outerCircleSize = 0.08;
        float innerCircleSize = 0.01;
        float distToCenter = distance(vRelPos, vec2(0.5, 0.5));
        bool isLine = abs(vRelPos.x - 0.5) < lineThickness / 2;
        isLine = isLine || min(min(vRelPos.x, 1-vRelPos.x), min(vRelPos.y, 1-vRelPos.y)) < lineThickness;
        bool isCenterCircle = distToCenter < outerCircleSize && distToCenter > outerCircleSize - lineThickness;
        isCenterCircle = isCenterCircle || distToCenter < innerCircleSize;
        if (isLine || isCenterCircle)
            fColor = 0.8 * fColor + 0.2 * vec3(1,1,1);
    }
}
