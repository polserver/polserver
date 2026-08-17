FROM trafex/php-nginx:latest

# Temporary switch to root
USER root

# The base image's PHP version moves with its :latest tag, and both the xsl package and the
# conf.d it drops its own extension= line into are named after that version, so derive it
# rather than hardcoding it. The build then fails here rather than at request time if the
# extension did not end up loaded: without it every XSLT-rendered page is a 500.
RUN PHP_V=$(php -r 'echo PHP_MAJOR_VERSION . PHP_MINOR_VERSION;') && \
    apk add --no-cache "php${PHP_V}-xsl" && \
    printf 'display_errors = on\n' > "/etc/php${PHP_V}/conf.d/settings.ini" && \
    php -m | grep -qx xsl

# Switch back to non-root user
USER nobody
