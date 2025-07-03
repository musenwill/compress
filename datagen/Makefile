
build:
	go build -o datagen

generate: build
	./datagen --out=../dataset/gen/1.raw random --num=1000000 --min=-128 --max=127
	./datagen --out=../dataset/gen/2.raw random --num=1000000 --min=-1073741824 --max=1073741824
	./datagen --out=../dataset/gen/3.raw repeat --num=1000000 --min=0 --max=127 --repeat=900000
	./datagen --out=../dataset/gen/4.raw repeat --num=1000000 --min=0 --max=1073741824 --repeat=900000
	./datagen --out=../dataset/gen/5.raw series --num=1000000 --start=1 --func=sin
	./datagen --out=../dataset/gen/6.raw series --num=1000000 --start=1 --func=linear --arg=1
	./datagen --out=../dataset/gen/7.raw series --num=1000000 --start=1 --func=linear --arg=16
	./datagen --out=../dataset/gen/8.raw series --num=1000000 --start=1 --func=square
	./datagen --out=../dataset/gen/9.raw cluster --num=1000000 --min=0 --max=72057594037927900 --subnum=900000 --submin=10 --submax=127
	./datagen --out=../dataset/gen/10.raw cluster --num=1000000 --min=0 --max=72057594037927900 --subnum=1000000 --submin=0 --submax=127

clean:
	rm -rf datagen
