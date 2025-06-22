SRCS = chess.cpp Board.cpp PieceFactory.cpp \
       Pawn.cpp Rook.cpp Knight.cpp Bishop.cpp Queen.cpp King.cpp

CXX = g++
TARGET = chess
WXCONFIG = wx-config

CFLAGS = $(shell $(WXCONFIG) --cxxflags) -std=c++17 -pthread
LIBS = $(shell $(WXCONFIG) --libs) -pthread

all:
	$(CXX) -o $(TARGET) $(SRCS) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)
