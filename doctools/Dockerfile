FROM trafex/php-nginx:latest

# Temporary switch to root
USER root

# Install xdebug
RUN apk add --no-cache php83-xsl

RUN echo "display_errors = on\nextension = php_xsl" > /etc/php83/conf.d/settings.ini

# Switch back to non-root user
USER nobody
