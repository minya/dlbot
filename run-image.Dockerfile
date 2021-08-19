FROM dlbot_build_img:latest as build

ADD ./src /code/

WORKDIR /code

RUN cmake
RUN cmake -DCMAKE_BUILD_TYPE=Release .
RUN cmake --build .

# Запуск ---------------------------------------

# В качестве базового образа используем ubuntu:latest
FROM ubuntu:latest

# Добавим пользователя, потому как в Docker по умолчанию используется root
# Запускать незнакомое приложение под root'ом неприлично :)
RUN groupadd -r sample && useradd -r -g sample sample
USER sample

# Установим рабочую директорию нашего приложения
WORKDIR /app

# Скопируем приложение со сборочного контейнера в рабочую директорию
COPY --from=build /code/dlbot .

# Установим точку входа
ENTRYPOINT ["./dlbot"]
