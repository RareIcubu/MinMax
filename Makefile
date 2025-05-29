SRCS = chess.cpp Board.cpp PieceFactory.cpp \
       Pawn.cpp Rook.cpp Knight.cpp Bishop.cpp Queen.cpp King.cpp

CXX = g++
TARGET = chess
WXCONFIG = wx-config

CFLAGS = $(shell $(WXCONFIG) --cxxflags)
LIBS = $(shell $(WXCONFIG) --libs)

all:
	$(CXX) -o $(TARGET) $(SRCS) $(CFLAGS) $(LIBS) -std=c++17

clean:
	rm -f $(TARGET)
