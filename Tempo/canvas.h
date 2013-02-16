// class used for drawing shit
class Canvas {
	private:
    int width;
    int height;

	public:
		Canvas();

    void initCanvas(); // inits GL
    void cleanupCanvas(); // cleans up GL
		void draw();
};
