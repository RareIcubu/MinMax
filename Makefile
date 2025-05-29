CXX = g++
WXCONFIG = wx-config
TARGET = chess
SRCS = chess.cpp Board.cpp  # Removed Pawn.cpp

CFLAGS = $(shell $(WXCONFIG) --cxxflags)
LIBS = $(shell $(WXCONFIG) --libs)

all:
	$(CXX) -o $(TARGET) $(SRCS) $(CFLAGS) $(LIBS) -std=c++17

clean:
	rm -f $(TARGET)
