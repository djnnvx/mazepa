FROM golang:1.23-alpine AS builder

WORKDIR /app
RUN apk update --no-cache && \
    apk upgrade --no-cache && \
    apk add --no-cache \
    make \
    build-base

COPY go.mod .
RUN go mod download

COPY . .
RUN make

ENTRYPOINT [ "/app/mazepa-server" ]
