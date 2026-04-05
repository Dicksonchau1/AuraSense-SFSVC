import type { HeroContent } from '../../types/content';
import { Button } from '../ui/Button';

interface HeroProps {
  content: HeroContent;
  large?: boolean;
}

export function Hero({ content, large = false }: HeroProps) {
  const headlineWords = content.headline.split(' ');

  return (
    <section
      className={`flex items-center justify-center text-center ${
        large ? 'min-h-[80vh]' : 'py-16 lg:py-24'
      }`}
      style={large ? { perspective: '1200px' } : undefined}
    >
      <div className={`max-w-4xl mx-auto px-6 ${large ? 'hero-3d' : 'hero-animate'}`}>
        {content.subheadline && (
          <p className="text-accent text-xs font-semibold tracking-[0.2em] uppercase mb-5">
            {content.subheadline}
          </p>
        )}
        <h1
          className={`text-text-primary font-bold leading-[1.1] tracking-tight ${
            large
              ? 'text-4xl md:text-5xl lg:text-6xl'
              : 'text-3xl md:text-4xl lg:text-5xl'
          }`}
        >
          {large ? (
            headlineWords.map((word, i) => (
              <span
                key={i}
                className="hero-3d-word"
                style={{ animationDelay: `${0.3 + i * 0.18}s` }}
              >
                {word}{' '}
              </span>
            ))
          ) : (
            content.headline
          )}
        </h1>
        <p className="text-text-secondary text-base md:text-lg max-w-2xl mx-auto mt-5 leading-relaxed">
          {content.description}
        </p>
        <div className="mt-8 flex flex-wrap items-center justify-center gap-4">
          <Button variant="primary" size="lg" href={content.primaryCta.href}>
            {content.primaryCta.label}
          </Button>
          {content.secondaryCta && (
            <Button variant="secondary" size="lg" href={content.secondaryCta.href}>
              {content.secondaryCta.label}
            </Button>
          )}
        </div>
      </div>
    </section>
  );
}
