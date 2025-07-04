
build:
	go build -o datagen

generate: build
	./datagen --out=../dataset/gen/1.raw random --num=1000000 --min=-128 --max=127
	./datagen --out=../dataset/gen/2.raw random --num=1000000 --min=-1073741824 --max=1073741824
	./datagen --out=../dataset/gen/3.raw repeat --num=1000000 --min=0 --max=127 --repeat=900000  --repeatVal=15
	./datagen --out=../dataset/gen/4.raw repeat --num=1000000 --min=0 --max=127 --repeat=850000  --repeatVal=15
	./datagen --out=../dataset/gen/5.raw repeat --num=1000000 --min=0 --max=127 --repeat=800000  --repeatVal=15
	./datagen --out=../dataset/gen/6.raw repeat --num=1000000 --min=0 --max=1073741824 --repeat=900000 --repeatVal=536870912
	./datagen --out=../dataset/gen/7.raw repeat --num=1000000 --min=0 --max=1073741824 --repeat=850000  --repeatVal=536870912
	./datagen --out=../dataset/gen/7.raw repeat --num=1000000 --min=0 --max=1073741824 --repeat=800000  --repeatVal=536870912
	./datagen --out=../dataset/gen/8.raw repeat --num=1000000 --min=0 --max=1073741824 --repeat=900000  --repeatVal=15
	./datagen --out=../dataset/gen/8.raw repeat --num=1000000 --min=0 --max=1073741824 --repeat=850000  --repeatVal=15
	./datagen --out=../dataset/gen/8.raw repeat --num=1000000 --min=0 --max=1073741824 --repeat=800000  --repeatVal=15
	./datagen --out=../dataset/gen/9.raw series --num=1000000 --start=1 --func=sin
	./datagen --out=../dataset/gen/10.raw series --num=1000000 --start=1 --func=linear --arg=1
	./datagen --out=../dataset/gen/11.raw series --num=1000000 --start=1 --func=linear --arg=16
	./datagen --out=../dataset/gen/12.raw series --num=1000000 --start=1 --func=square
	./datagen --out=../dataset/gen/13.raw cluster --num=1000000 --min=0 --max=72057594037927900 --subnum=1000000 --submin=10 --submax=127
	./datagen --out=../dataset/gen/14.raw cluster --num=1000000 --min=0 --max=72057594037927900 --subnum=900000 --submin=10 --submax=127
	./datagen --out=../dataset/gen/15.raw cluster --num=1000000 --min=0 --max=72057594037927900 --subnum=800000 --submin=10 --submax=127

clean:
	rm -rf datagen
